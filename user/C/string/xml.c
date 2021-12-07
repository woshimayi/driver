#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef signed char    INT8;
typedef signed short   INT16;
typedef signed long    INT32;
#define SINT32 int

typedef struct nxml *nxml_t;

typedef enum
{
    state_start,    /* look for <? */
    state_begin_tag, /* look for < or data */
    state_tag_name, /* found <, looking for whole name */
    state_end_tag_name, /* found </, looking for whole name */
    state_attr_name, /* tag begun, looking for attr */
    state_attr_value_equals, /* attr_name found, looking for = */
    state_attr_value_quote, /* attr_name and = found, looking for quote */
    state_attr_value, /* attr name found, sending attr_value */
    state_finish_tag /* look for the >, ignoring everything else */
} nxml_state;


typedef struct
{
    void (*tag_begin)(nxml_t handle, const char *tag_name, unsigned len);
    void (*attribute_begin)(nxml_t handle, const char *attr_name, unsigned len);
    void (*attribute_value)(nxml_t handle, const char *attr_value, unsigned len, int more);
    void (*data)(nxml_t handle, const char *data, unsigned len, int more);
    void (*tag_end)(nxml_t handle, const char *tag_name, unsigned len);
} nxml_settings;

typedef struct NameSpace
{
    char    *rcvPrefix;     /* pointers to prefix names: set by each envelope */
    /* xmlns: attribute list */
    char    *sndPrefix;     /* prefixs to use on sent msgs */
    char    *nsURL;         /* namespace URL for this application  */
    /* as defined by DSL Forum TR-069 */
} NameSpace;

typedef enum
{
    TOKEN_INVALID,
    TAGBEGIN,
    TAGEND,
    TAGDATA,
    ATTRIBUTE,
    ATTRIBUTEVALUE
} TOKEN_TYPE;

typedef XML_STATUS(*XML_SET_FUNC)(const char *name, TOKEN_TYPE ttype, const char *value);

typedef struct XmlNodeDesc
{
    NameSpace    *nameSpace;
    char         *tagName;
    XML_SET_FUNC setXmlFunc;
    void        *leafNode;
} XmlNodeDesc;



struct nxml
{
    nxml_settings settings;
    nxml_state state;
    char namecache[128];
    int namecachesize;
    int skipwhitespace;
    int treelevel;
    int nodeFlags;
    NameSpace   *nameSpaces;    /* pointer to nameSpace tables */
    void (*scanSink)(TOKEN_TYPE ttype, const char *data);
    void (*embeddedDataSink)(const char *data, int len);
    void (*parse_error)(char *errfmt, ...);
    XmlNodeDesc     *node;      /* points to the current node */
    int             level;      /* xml level - starting at 0 */
    char            attrName[128];
    XmlNodeDesc     *nodestack[20];  /* points at next higher node */
    XmlNodeDesc     *itemstack[20];  /* points at item used at node*/
    char	    *valueptr;		/* Accumlated attr value */
    int		    valuelth;		/* lth of accumlated attr value */
    char        *dataptr;       /* Accumlated data */
    int         datalth;        /* lth of accumlated data*/
};


typedef struct
{
    void (*tag_begin)(nxml_t handle, const char *tag_name, unsigned len);
    void (*attribute_begin)(nxml_t handle, const char *attr_name, unsigned len);
    void (*attribute_value)(nxml_t handle, const char *attr_value, unsigned len, int more);
    void (*data)(nxml_t handle, const char *data, unsigned len, int more);
    void (*tag_end)(nxml_t handle, const char *tag_name, unsigned len);
} nxml_settings;

/* config write related functions */
static void mdm_tagBeginCallbackFunc(nxml_t handle, const char *tagName, UINT32 len);
static void mdm_attrBeginCallbackFunc(nxml_t handle, const char *attrName, UINT32 len);
static void mdm_attrValueCallbackFunc(nxml_t handle, const char *attrValue, UINT32 len, SINT32 more);
static void mdm_dataCallbackFunc(nxml_t handle, const char *data, UINT32 len, SINT32 more);
static void mdm_tagEndCallbackFunc(nxml_t handle, const char *tagName, UINT32 len);

static void recoverFactoryParam(UBOOL8 isRecover);


void mdm_tagBeginCallbackFunc(nxml_t handle __attribute__((unused)),
                              const char *tagName,
                              UINT32 len)
{
    char buf[NXML_MAX_NAME_SIZE + 1] = {0};
    MdmObjectNode *objNode = NULL;
    MdmParamNode *paramNode = NULL;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        if (!nxmlCtx.topNodeFound || !nxmlCtx.versionFound)
        {
            return;
        }

        /* don't traverse any more if error is detected. */
        nxmlCtx.ret = VOS_RET_SUCCESS;
    }

    UTIL_STRNCPY(buf, tagName, len + 1);

    if (SF_FEATURE_SUPPORT_CONFIG_COMPAT)
    {
        /************** Begin convert v1,v2 config file ****************/

        /*
        * In v1, v2, the Time object had X_BROADCOM_COM_NTPEnable.
        * In v3, we use the BBF defined Enable parameter.
        */
        if ((nxmlCtx.versionMajor < 3)
                && (nxmlCtx.objNode != NULL)
                && (MDMOID_TIME_SERVER_CFG == nxmlCtx.objNode->oid)
                && (!util_strcmp(buf, "X_BROADCOM_COM_NTPEnable")))
        {
            UTIL_SNPRINTF(buf, len + 1, "Enable");
            vosLog_debug("converted X_BROADCOM_COM_NTPEnable to %s", buf);
        }

        /*
        * In v2 and v2, there was a WanPPPConnection.X_BROADCOM_COM_BcastAddr.
        * In v3, we deleted that parameter.  So just pretend the current param
        * is X_BROADCOM_COM_IfName so that we can have a paramNode.  We won't do
        * anything to the X_BROADCOM_COM_IfName param though, we just need to
        * point to a similar string type param node so that later processing will
        * not get confused.
        */
        if ((nxmlCtx.versionMajor < 3)
                && (nxmlCtx.objNode != NULL)
                && (MDMOID_WAN_PPP_CONN == nxmlCtx.objNode->oid)
                && (!util_strcmp(buf, "X_BROADCOM_COM_BcastAddr")))
        {
            vosLog_debug("fake X_BROADCOM_COM_BcastAddr to X_BROADCOM_COM_IfName");
            UTIL_SNPRINTF(buf, len + 1, "X_BROADCOM_COM_IfName");
        }

        /**************** end config file conversion ****************/
    }


    if (nxmlCtx.objNode == NULL)
    {
        /*
         * This is very early in the config file.  It must start with the
         * CpeConfigFile node followed by the InternetGatewayDevice node.
         */
        if (!util_strcmp(buf, CONFIG_FILE_TOP_NODE))
        {
            if (nxmlCtx.topNodeFound)
            {
                vosLog_error("multiple %s nodes detected", CONFIG_FILE_TOP_NODE);
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else
            {
                vosLog_debug("%s node detected", CONFIG_FILE_TOP_NODE);
                nxmlCtx.topNodeFound = TRUE;
            }
        }
        else if (!util_strcmp(buf, mdmShmCtx->rootObjNode->name))
        {
            vosLog_debug("%s node detected", mdmShmCtx->rootObjNode->name);
            nxmlCtx.objNode = mdmShmCtx->rootObjNode;

            ret = mdm_getDefaultObject(nxmlCtx.objNode->oid, &(nxmlCtx.mdmObj));
            if (ret != VOS_RET_SUCCESS)
            {
                nxmlCtx.ret = ret;
            }
        }
        else if (!util_strcmp(buf, CONFIG_FILE_PSI_TOP_NODE))
        {
            vosLog_notice("PSI top node detected");
            nxmlCtx.ret = VOS_RET_CONFIG_PSI;
        }
        else
        {
            vosLog_error("Invalid start node %s", buf);
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }

        return;
    }


    /*
     * a tag can start a child object node, a peer object node,
     * or a parameter node of the current object node.
     */
    if (((objNode = mdm_getChildObjectNode(nxmlCtx.objNode, buf)) != NULL) ||
            ((nxmlCtx.gotCurrObjEndTag == TRUE) &&
             (nxmlCtx.objNode->parent != NULL) &&
             ((objNode = mdm_getChildObjectNode(nxmlCtx.objNode->parent, buf)) != NULL)))
    {

        if (nxmlCtx.objNode == objNode->parent)
        {
            vosLog_debug("%s --> child obj %s", nxmlCtx.objNode->name, objNode->name);
        }
        else
        {
            vosLog_debug("%s --> peer obj %s", nxmlCtx.objNode->name, objNode->name);
        }

        nxmlCtx.gotCurrObjEndTag = FALSE;

        if (nxmlCtx.loadMdm)
        {
            /*
             * Since we are transitioning away from the current object,
             * set any attributes of the current objNode sub-tree that we detected.
             */
            if (nxmlCtx.attr.accessBitMaskChange || nxmlCtx.attr.notificationChange)
            {
                vosLog_error("set sub-tree %s %s attr notif=(%d)%d access=(%d)0x%x",
                             nxmlCtx.objNode->name,
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                             nxmlCtx.attr.notificationChange,
                             nxmlCtx.attr.notification,
                             nxmlCtx.attr.accessBitMaskChange,
                             nxmlCtx.attr.accessBitMask);

                ret = mdm_setSubTreeParamAttributes(nxmlCtx.objNode,
                                                    &(nxmlCtx.iidStack),
                                                    &(nxmlCtx.attr),
                                                    FALSE);
                if (ret != VOS_RET_SUCCESS)
                {
                    vosLog_error("setSubTree attr failed for %s %s notification=(%d)%d accessBitMask=(%d)0x%x ret=%d",
                                 nxmlCtx.objNode->name,
                                 mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                                 nxmlCtx.attr.notificationChange,
                                 nxmlCtx.attr.notification,
                                 nxmlCtx.attr.accessBitMaskChange,
                                 nxmlCtx.attr.accessBitMask,
                                 ret);
                    nxmlCtx.ret = ret;
                }

                nxmlCtx.attr.accessBitMaskChange = 0;
                nxmlCtx.attr.notificationChange = 0;
            }


            /*
             * Push new object into MDM.  We could optimize this code to detect
             * if any changes have actually been made to the mdmObj so we
             * don't end up pushing the exact same object into the MDM.
             *
             * Every new value we've put into the mdmObj has been validated
             * in the dataCallbackFunc, so there is no need to validate the
             * entire mdmObj here.
             *
             * This one catches the case where we are transitioning further down
             * into the MDM hierarchy.
             */
            if (nxmlCtx.mdmObj != NULL && nxmlCtx.nextInstanceNode == FALSE)
            {
                vosLog_debug("setting obj %s %s",
                             mdm_oidToGenericPath(*((MdmObjectId *) nxmlCtx.mdmObj)),
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                ret = mdm_setObject(&(nxmlCtx.mdmObj), &(nxmlCtx.iidStack), FALSE);
                if (ret != VOS_RET_SUCCESS)
                {
                    vosLog_error("setObject failed, %d", ret);
                    mdm_freeObject(&(nxmlCtx.mdmObj));
                    nxmlCtx.ret = ret;
                }
            }
        }
        else
        {
            /* not loading mdm, just free the mdmObj */
            if (nxmlCtx.mdmObj)
            {
                mdm_freeObject(&(nxmlCtx.mdmObj));
            }
        }


        nxmlCtx.nextInstanceNode = FALSE;


        /* record the new object node that we are working on. */
        nxmlCtx.objNode = objNode;


        /*
         * node's attributes are inheritied from the parent (I don't think I
         * need this line below.  Any changes to attributes should have
         * been written out in the config file.)
         */
        nxmlCtx.attr = objNode->parent->nodeAttr;


        /* start a new MdmObject */
        ret = mdm_getDefaultObject(nxmlCtx.objNode->oid, &(nxmlCtx.mdmObj));
        if (ret != VOS_RET_SUCCESS)
        {
            nxmlCtx.ret = ret;
        }
    }
    else if ((paramNode = mdm_getParamNode(nxmlCtx.objNode->oid, buf)) != NULL)
    {
        vosLog_debug("%s :: param %s", nxmlCtx.objNode->name, paramNode->name);

        if (nxmlCtx.nextInstanceNode)
        {
            vosLog_error("param node %s detected under a next instance object", paramNode->name);
            nxmlCtx.paramNode = paramNode;
            return;
        }

        /*
         * We could be transitioning from obj to param, if any attribute
         * has changed, set them for the current objNode sub-tree.
         * The fourth parameter of setSubTreeParamAttributes is testOnly,
         * so we pass in !loadMdm, which which means when loadMdm=FALSE, testOnly=TRUE.
         */
        if ((nxmlCtx.loadMdm) &&
                (nxmlCtx.attr.accessBitMaskChange || nxmlCtx.attr.notificationChange))
        {
            vosLog_error("set sub-tree %s %s attr notif=(%d)%d access=(%d)0x%x",
                         nxmlCtx.objNode->name,
                         mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                         nxmlCtx.attr.notificationChange,
                         nxmlCtx.attr.notification,
                         nxmlCtx.attr.accessBitMaskChange,
                         nxmlCtx.attr.accessBitMask);

            ret = mdm_setSubTreeParamAttributes(nxmlCtx.objNode,
                                                &(nxmlCtx.iidStack),
                                                &(nxmlCtx.attr),
                                                FALSE);
            if (ret != VOS_RET_SUCCESS)
            {
                vosLog_error("setSubTree attr failed for %s %s notification=(%d)%d accessBitMask=(%d)0x%x ret=%d",
                             nxmlCtx.objNode->name,
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                             nxmlCtx.attr.notificationChange,
                             nxmlCtx.attr.notification,
                             nxmlCtx.attr.accessBitMaskChange,
                             nxmlCtx.attr.accessBitMask,
                             ret);
                nxmlCtx.ret = ret;
            }

            nxmlCtx.attr.accessBitMaskChange = 0;
            nxmlCtx.attr.notificationChange = 0;
        }


        if (nxmlCtx.paramNode != NULL)
        {
            vosLog_error("embedded param node detected %s %s",
                         nxmlCtx.paramNode->name, paramNode->name);
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }
        else
        {
            nxmlCtx.paramNode = paramNode;
        }
    }
    else
    {
        vosLog_error("Unrecognized tag %s\n", buf);
        nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
    }

    return;
}

static void mdm_attrBeginCallbackFunc(nxml_t handle __attribute__((unused)),
                                      const char *attrName,
                                      UINT32 len)
{
    char buf[NXML_MAX_NAME_SIZE + 1];

    UTIL_STRNCPY(buf, attrName, len + 1);

    if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        /* don't traverse any more if error has been detected. */
        return;
    }

    vosLog_debug("%s", buf);

    if (nxmlCtx.currXmlAttr != MDM_CONFIG_XMLATTR_NONE)
    {
        vosLog_error("overlapping attrs, currXmlAttr=%d", nxmlCtx.currXmlAttr);
        nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        return;
    }

    if (!util_strcmp(CONFIG_FILE_ATTR_VERSION, buf))
    {
        nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_VERSION;
    }
    else if (!util_strcmp(CONFIG_FILE_ATTR_INSTANCE, buf))
    {
        nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_INSTANCE;
    }
    else if (!util_strcmp(CONFIG_FILE_ATTR_NEXT_INSTANCE, buf))
    {
        nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_NEXT_INSTANCE;
    }
    else if (!util_strcmp(CONFIG_FILE_ATTR_ACCESS_LIST, buf))
    {
        nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_ACCESS_LIST;
    }
    else if (!util_strcmp(CONFIG_FILE_ATTR_NOTIFICATION, buf))
    {
        nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_NOTIFICATION;
    }
    else
    {
        vosLog_error("Unrecognized attribute %s", buf);
        nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
    }

    return;
}


static void mdm_attrValueCallbackFunc(nxml_t handle __attribute__((unused)),
                                      const char *attrValue,
                                      UINT32 len,
                                      SINT32 more)
{
    char *buf;

    if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        /* don't traverse any more if error has been detected. */
        return;
    }

    /*
     * our attribute values are always written out in a single line,
     * so more should always be 0.
     */
    if (more != 0)
    {
        vosLog_error("multi-line attribute value detected");
        nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        return;
    }

    if ((buf = VOS_MALLOC_FLAGS(len + 1, ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("malloc of %d bytes failed", len + 1);
        nxmlCtx.ret = VOS_RET_RESOURCE_EXCEEDED;
        return;
    }

    UTIL_STRNCPY(buf, attrValue, len + 1);

    vosLog_debug("(more=%d)%s", more, buf);

    switch (nxmlCtx.currXmlAttr)
    {
        case MDM_CONFIG_XMLATTR_VERSION:
        {
            /*
             * Config file version may help us with conversion later on.
             * For now, just print it out.
             */
            if (nxmlCtx.objNode != NULL)
            {
                vosLog_error("version attr can only appear in the top node");
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else
            {
                vosLog_debug("config file version=%s", buf);
                nxmlCtx.ret = parseVersionNumber(buf, &nxmlCtx.versionMajor, &nxmlCtx.versionMinor);

                if (nxmlCtx.ret == VOS_RET_SUCCESS)
                {
                    nxmlCtx.versionFound = TRUE;

                    parseVersionNumber(CMC_CONFIG_FILE_VERSION, &myMajor, &myMinor);

                    if (SF_FEATURE_SUPPORT_CONFIG_COMPAT)
                    {
                        /*
                                    * If CMC_CONFIG_COMPAT is defined, then we can accept any version
                                    * of the config file that is less than or equal to my current version.
                                    */
                        if (nxmlCtx.versionMajor > myMajor)
                        {
                            vosLog_error("config file version number is %d, this software only supports up to %d",
                                         nxmlCtx.versionMajor, myMajor);
                            nxmlCtx.ret = VOS_RET_INVALID_CONFIG_FILE;
                        }
                    }
                    else
                    {
                        /*
                                    * If CMC_CONFIG_COMPAT is not defined, then we can only accept
                                    * config files that have exactly the same version as my current version.
                                    */
                        if (nxmlCtx.versionMajor != myMajor)
                        {
                            vosLog_error("config file version number is %d, this software only supports %d",
                                         nxmlCtx.versionMajor, myMajor);
                            vosLog_error("You may need to enable backward compatibility of CMS Config files in make menuconfig");
                            nxmlCtx.ret = VOS_RET_INVALID_CONFIG_FILE;
                        }
                    }
                }
            }
            break;
        }

        case MDM_CONFIG_XMLATTR_INSTANCE:
        {
            UINT32 instanceId;
            VOS_RET_E ret;

            if ((ret = util_strtoul(buf, NULL, 0, &instanceId)) != VOS_RET_SUCCESS)
            {
                vosLog_error("invalid instance number %s", buf);
                nxmlCtx.ret = ret;
            }
            else if (!(IS_INDIRECT2(nxmlCtx.objNode)))
            {
                vosLog_error("got instance number on non-indirect2 node, %s",
                             nxmlCtx.objNode->name);
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else if (DEPTH_OF_IIDSTACK(&(nxmlCtx.iidStack)) + 1 != nxmlCtx.objNode->instanceDepth)
            {
                vosLog_error("instance depth mismatch on %s, instance depth=%d iidStack %s",
                             nxmlCtx.objNode->name,
                             nxmlCtx.objNode->instanceDepth,
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else
            {
                MdmNodeAttributes parentAttr;

                /*
                 * Get the parent node's attributes.  The newly created sub-tree
                 * will inherit these attributes.  But be careful, the location
                 * of the parent's attributes depend on the type of node it is.
                 */
                if (nxmlCtx.loadMdm)
                {
                    if (IS_INDIRECT0(nxmlCtx.objNode->parent))
                    {
                        parentAttr = nxmlCtx.objNode->parent->nodeAttr;
                    }
                    else if (IS_INDIRECT1(nxmlCtx.objNode->parent))
                    {
                        InstanceHeadNode *instHead;

                        instHead = mdm_getInstanceHead(nxmlCtx.objNode->parent, &(nxmlCtx.iidStack));
                        if (instHead == NULL)
                        {
                            vosLog_error("could not find instHead for %p %s",
                                         nxmlCtx.objNode->parent,
                                         mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
                        }
                        else
                        {
                            parentAttr = instHead->nodeAttr;
                        }
                    }
                    else
                    {
                        /* must be indirect 2 */
                        InstanceDescNode *instDesc;

                        instDesc = mdm_getInstanceDescFromObjNode(nxmlCtx.objNode->parent,
                                   &(nxmlCtx.iidStack));
                        if (instDesc == NULL)
                        {
                            vosLog_error("could not find instDesc for %s %s",
                                         nxmlCtx.objNode->parent->name,
                                         mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
                        }
                        else
                        {
                            parentAttr = instDesc->nodeAttr;
                        }
                    }
                }


                /* create the instance in the MDM */
                PUSH_INSTANCE_ID(&(nxmlCtx.iidStack), instanceId);
                vosLog_debug("Got instanceId for indirect 2 node %s %s",
                             mdm_oidToGenericPath(nxmlCtx.objNode->oid),
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)));

                if (nxmlCtx.loadMdm)
                {
                    MdmPathDescriptor pathDesc = EMPTY_PATH_DESCRIPTOR;
                    pathDesc.oid = nxmlCtx.objNode->oid;
                    pathDesc.iidStack = nxmlCtx.iidStack;
                    if (FALSE == mdm_isPathDescriptorExist(&pathDesc))
                    {
                        ret = mdm_createSubTree(nxmlCtx.objNode,
                                                nxmlCtx.objNode->instanceDepth,
                                                &(nxmlCtx.iidStack),
                                                NULL, NULL);
                    }
                    if (ret != VOS_RET_SUCCESS)
                    {
                        vosLog_error("create subtree for obj %s %s failed, ret=%d",
                                     nxmlCtx.objNode->name,
                                     mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                                     ret);
                        nxmlCtx.ret = ret;
                    }
                    else
                    {
                        parentAttr.notificationChange = 1;
                        parentAttr.accessBitMaskChange = 1;

                        vosLog_debug("set sub-tree %s %s attr notif=(%d)%d access=(%d)0x%x",
                                     nxmlCtx.objNode->name,
                                     mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                                     parentAttr.notificationChange,
                                     parentAttr.notification,
                                     parentAttr.accessBitMaskChange,
                                     parentAttr.accessBitMask);


                        ret = mdm_setSubTreeParamAttributes(nxmlCtx.objNode,
                                                            &(nxmlCtx.iidStack),
                                                            &parentAttr,
                                                            FALSE);
                        if (ret != VOS_RET_SUCCESS)
                        {
                            vosLog_error("setSubTree attr failed for %s %s notification=(%d)%d accessBitMask=(%d)0x%x, ret=%d",
                                         nxmlCtx.objNode->name,
                                         mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                                         parentAttr.notificationChange,
                                         parentAttr.notification,
                                         parentAttr.accessBitMaskChange,
                                         parentAttr.accessBitMask,
                                         ret);
                            nxmlCtx.ret = ret;
                        }
                    }

                }
            }

            break;
        }

        case MDM_CONFIG_XMLATTR_NEXT_INSTANCE:
        {
            UINT32 nextInstanceIdToAssign;
            VOS_RET_E ret;

            if ((ret = util_strtoul(buf, NULL, 0, &nextInstanceIdToAssign)) != VOS_RET_SUCCESS)
            {
                vosLog_error("invalid next instance number %s", buf);
                nxmlCtx.ret = ret;
            }
            else if (!(IS_INDIRECT2(nxmlCtx.objNode)))
            {
                vosLog_error("got next instance number on non-indirect2 node, %s",
                             nxmlCtx.objNode->name);
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else if (DEPTH_OF_IIDSTACK(&(nxmlCtx.iidStack)) + 1 != nxmlCtx.objNode->instanceDepth)
            {
                vosLog_error("instance depth mismatch on %s, instance depth=%d iidStack %s",
                             nxmlCtx.objNode->name,
                             nxmlCtx.objNode->instanceDepth,
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else
            {

                nxmlCtx.nextInstanceNode = TRUE;

                /*
                 * Just set the nextInstanceId here.  We know there are no other
                 * attributes or parameters that we need to collect for this object.
                 */
                if (nxmlCtx.loadMdm)
                {
                    InstanceHeadNode *instHead;

                    vosLog_debug("setting nextInstanceId to %u for objNode %s iidStack=%s",
                                 nextInstanceIdToAssign, nxmlCtx.objNode->name, mdm_dumpIidStack(&(nxmlCtx.iidStack)));

                    if ((instHead = mdm_getInstanceHead(nxmlCtx.objNode, &(nxmlCtx.iidStack))) == NULL)
                    {
                        vosLog_error("could not find instHead for %s %s", nxmlCtx.objNode->name, mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                    }
                    else
                    {
                        instHead->nextInstanceIdToAssign = nextInstanceIdToAssign;
                    }
                }

                /*
                 * Even though we don't need to do anything with the iidStack here,
                 * push an instance id on the iidStack now because when we hit the
                 * end tag, we will pop it off.
                 */
                PUSH_INSTANCE_ID(&(nxmlCtx.iidStack), 1);
                vosLog_debug("pushed fake iidStack, now is %s", mdm_dumpIidStack(&(nxmlCtx.iidStack)));
            }

            break;
        }


        case MDM_CONFIG_XMLATTR_ACCESS_LIST:
        {
            UINT16 accessBitMask = 0;
            VOS_RET_E ret;

            ret = vosEid_getBitMaskFromStringNames(buf, &accessBitMask);
            if (ret != VOS_RET_SUCCESS)
            {
                vosLog_error("conversion of %s to accessBitMask failed, ret=%d",
                             buf, ret);
                nxmlCtx.ret = ret;
            }
            else
            {
                nxmlCtx.attr.accessBitMaskChange = 1;
                nxmlCtx.attr.accessBitMask = accessBitMask;
            }

            /* we need to do more when we are acutally pushing data into MDM */
            break;
        }

        case MDM_CONFIG_XMLATTR_NOTIFICATION:
        {
            UINT32 notification;
            VOS_RET_E ret;

            ret = util_strtoul(buf, NULL, 0, &notification);

            if ((ret != VOS_RET_SUCCESS) ||
                    ((notification != NDA_TR69_NO_NOTIFICATION) &&
                     (notification != NDA_TR69_PASSIVE_NOTIFICATION) &&
                     (notification != NDA_TR69_ACTIVE_NOTIFICATION) &&
                     SF_AND_IF(SF_FEATURE_SUPPORT_CT_MIDDLEWARE)
                     (notification != NDA_ITMS_NOTIFICATION_OFF_WRITABLE)
                     && (notification != NDA_ITMS_NOTIFICATION_WRITABLE)
                     && (notification != NDA_ITMS_REBOOT_NOTIFICATION_WRITABLE)
                     && (notification != NDA_CTMDW_NOTIFICATION_READABLE)
                     && (notification != NDA_CTMDW_ITMS_NOTIFICATION_READABLE)
                     && (notification != NDA_CTMDW_REBOOT_ITMS_NOTIFICATION_READABLE)
                     && (notification != NDA_CTMDW_NOTIFICATION_WRITABLE)
                     && (notification != NDA_CTMDW_ITMS_NOTIFICATION_WRITABLE)
                     && (notification != NDA_CTMDW_REBOOT_ITMS_NOTIFICATIONW_WRITABLE)
                     SF_AND_ENDIF))
            {
                vosLog_error("invalid notification number %s", buf);
                nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            }
            else
            {
                nxmlCtx.attr.notificationChange = 1;
                nxmlCtx.attr.notification = notification;
            }

            break;
        }

        case MDM_CONFIG_XMLATTR_NONE:
        {
            vosLog_error("got attr value with no attr tag in progress.");
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
            break;
        }

        default:
        {
            vosLog_error("Unrecognized attribute %d", nxmlCtx.currXmlAttr);
            nxmlCtx.ret = VOS_RET_INTERNAL_ERROR;
            break;
        }

    } /* end of switch(nxmlCtx.currXmlAttr) */


    /* we are done with attr processing */
    nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_NONE;

    VOS_MEM_FREE_BUF_AND_NULL_PTR(buf);
}


static void mdm_dataCallbackFunc(nxml_t handle __attribute__((unused)),
                                 const char *data,
                                 UINT32 len,
                                 SINT32 more)
{

    if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        /* don't traverse any more if error is detected. */
        return;
    }

    /*
     * I don't know if we will ever have data that spans multiple
     * lines.  Don't implement this feature for now.  So this code
     * will only use the last line that was detected and drop
     * all previous lines.
     */
    if (more != 0)
    {
        vosLog_error("support for multiple data lines not implemented yet.");
        //nxmlCtx.ret = VOS_RET_INTERNAL_ERROR;
        return;
    }

    if (nxmlCtx.paramValue != NULL
            || NULL == nxmlCtx.paramNode
            || NULL == nxmlCtx.objNode)
    {
        vosLog_error("paramValue=%p, paramNode=%p, objNode=%p", nxmlCtx.paramValue, nxmlCtx.paramNode, nxmlCtx.objNode);
        //nxmlCtx.ret = VOS_RET_INTERNAL_ERROR;
        return;
    }

    /*
     * we are allocating data in heap memory.  If this data is for
     * a string param, mdm_setParamNodeString will make a copy in shared
     * memory and set that in the mdmObj.  If we have very large string
     * objects, we could optimize this by allocating from shared memory
     * here and let mdm_setParamNodeString steal our buffer.  For small string
     * value buffers, its no big deal.
     */
    if ((nxmlCtx.paramValue = VOS_MALLOC_FLAGS(len + 1, ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("malloc of %d bytes failed", len + 1);
    }
    else
    {
        UTIL_STRNCPY(nxmlCtx.paramValue, data, len + 1);

        vosLog_debug("(more=%d)%s", more, nxmlCtx.paramValue);

        /*
         * For string params, we need to unescape any XML character escape
         * sequences before sending to MDM.
         */
        if (nxmlCtx.paramNode->type == MPT_STRING)
        {
            char *unescapedString = NULL;

            utilXml_unescapeString(nxmlCtx.paramValue, &unescapedString);

            VOS_FREE(nxmlCtx.paramValue);
            nxmlCtx.paramValue = unescapedString;
        }
    }

    return;
}


void mdm_tagEndCallbackFunc(nxml_t handle __attribute__((unused)),
                            const char *tagName,
                            UINT32 len)
{
    char buf[NXML_MAX_NAME_SIZE + 1];

    UTIL_STRNCPY(buf, tagName, len + 1);

    if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        /* don't traverse any more if error is detected. */

        if ((!nxmlCtx.topNodeFound || !nxmlCtx.versionFound)
                || (NULL == nxmlCtx.paramNode && NULL != nxmlCtx.objNode && util_strcmp(nxmlCtx.objNode->name, buf)))
        {
            vosLog_error("Unrecognized tag %s\n", buf);
            return;
        }
    }

    if (nxmlCtx.paramNode != NULL)
    {
        /*
         * This is the end tag of a param.  The value of the param is in the
         * paramValue field.  Update my mdmObj.
         */

        if (SF_FEATURE_SUPPORT_CONFIG_COMPAT)
        {
            /************** Begin convert v1,v2 config file ****************/

            /*
                    * In v1, v2, the Time object had X_BROADCOM_COM_NTPEnable.
                    * In v3, we use the BBF defined Enable parameter.
                    */
            if ((nxmlCtx.versionMajor < 3)
                    && (nxmlCtx.objNode != NULL)
                    && (MDMOID_TIME_SERVER_CFG == nxmlCtx.objNode->oid)
                    && (!util_strcmp(buf, "X_BROADCOM_COM_NTPEnable")))
            {
                UTIL_SNPRINTF(buf, len + 1, "Enable");
                vosLog_debug("converted X_BROADCOM_COM_NTPEnable to %s", buf);
            }

            /*
                    * In v2 and v2, there was a WanPPPConnection.X_BROADCOM_COM_BcastAddr.
                    * In v3, we deleted that parameter, so just ignore it and return here.
                    */
            if ((nxmlCtx.versionMajor < 3)
                    && (nxmlCtx.objNode != NULL)
                    && (MDMOID_WAN_PPP_CONN == nxmlCtx.objNode->oid)
                    && (!util_strcmp(buf, "X_BROADCOM_COM_BcastAddr")))
            {
                vosLog_debug("ignore X_BROADCOM_COM_BcastAddr in WanPppConn");
                VOS_MEM_FREE_BUF_AND_NULL_PTR(nxmlCtx.paramValue);
                nxmlCtx.paramNode = NULL;
                return;
            }

            /**************** end config file conversion ****************/
        }

        if (util_strcmp(nxmlCtx.paramNode->name, buf))
        {
            vosLog_error("unexpected end tag, expected param name %s got %s",
                         nxmlCtx.paramNode->name, buf);
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }
        else if (nxmlCtx.mdmObj == NULL)
        {
            vosLog_error("no mdmObj but end param tag.");
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }
        else if ((nxmlCtx.paramValue == NULL) &&
                 ((nxmlCtx.paramNode->type != MPT_STRING) &&
                  (nxmlCtx.paramNode->type != MPT_DATE_TIME) &&
                  (nxmlCtx.paramNode->type != MPT_BASE64) &&
                  (nxmlCtx.paramNode->type != MPT_HEX_BINARY)))
        {
            /* non-string types must have a value */
            vosLog_error("no param value but end param tag.");
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }
        else
        {
            VOS_RET_E r2 = VOS_RET_SUCCESS;

            if (SF_FEATURE_SUPPORT_CONFIG_COMPAT)
            {
                /************* Begin convert v1 config file ****************/
                if ((nxmlCtx.versionMajor == 1)
                        && (!util_strcmp(nxmlCtx.paramNode->name, "AdminPassword")
                            || !util_strcmp(nxmlCtx.paramNode->name, "SupportPassword")
                            || !util_strcmp(nxmlCtx.paramNode->name, "UserPassword")
                            || (!util_strcmp(nxmlCtx.paramNode->name, "Password") && !util_strcmp(nxmlCtx.objNode->name, "WANPPPConnection"))))
                {
                    vosLog_notice("v1 password detected, paramNode->name=%s", nxmlCtx.paramNode->name);
                    convert_v1_password();
                }
                /************* End convert v1 config file ****************/

                /************* Begin convert v1,v2 config file ****************/
                if ((nxmlCtx.versionMajor < 3)
                        && (nxmlCtx.objNode->oid == MDMOID_L3_FORWARDING)
                        && (!util_strcmp(nxmlCtx.paramNode->name, "DefaultConnectionService"))
                        && (nxmlCtx.paramValue != NULL)
                        && (nxmlCtx.loadMdm))
                {
                    vosLog_debug("converting v1/v2 defaultConnectionService");
                    convert_v1_v2_defaultGateway();
                }

                /************* End convert v1,v2 config file ****************/
            }

            if (nxmlCtx.paramNode->flags & PRN_CONFIG_PASSWORD)
            {
                /* in some very weird corner cases (e.g. unit tests), password may be blank */
                if (nxmlCtx.paramValue)
                {
                    char *plaintext = NULL;
                    UINT32 plaintextLen;
                    r2 = utilB64_decode(nxmlCtx.paramValue, (unsigned char **) &plaintext, &plaintextLen);
                    if (r2 == VOS_RET_SUCCESS)
                    {
                        vosLog_debug("decoded string=%s", plaintext);
                        VOS_FREE(nxmlCtx.paramValue);
                        nxmlCtx.paramValue = plaintext;
                    }
                    else
                    {
                        vosLog_debug("utilB64_decode error, ret=%d, invalid value=%s for param=%s, used defaultValue=%s",
                                     r2, nxmlCtx.paramValue,
                                     nxmlCtx.paramNode->name,
                                     nxmlCtx.paramNode->defaultValue);
                        VOS_MEM_REPLACE_STRING(nxmlCtx.paramValue, nxmlCtx.paramNode->defaultValue);
                    }
                }
            }

            r2 = mdm_validateParamNodeString(nxmlCtx.paramNode, nxmlCtx.paramValue);
            if (r2 != VOS_RET_SUCCESS)
            {
                vosLog_error("invalid string %s for param %s(oid %d), used defaultValue %s",
                             nxmlCtx.paramValue,
                             nxmlCtx.paramNode->name,
                             nxmlCtx.objNode->oid,
                             nxmlCtx.paramNode->defaultValue);
                VOS_MEM_REPLACE_STRING(nxmlCtx.paramValue, nxmlCtx.paramNode->defaultValue);
            }

            r2 = mdm_setParamNodeString(nxmlCtx.paramNode, nxmlCtx.paramValue, mdmLibCtx.allocFlags, nxmlCtx.mdmObj);
            if (r2 != VOS_RET_SUCCESS)
            {
                vosLog_error("set obj %d :: param %s -> value %s error %d",
                             nxmlCtx.objNode->oid,
                             nxmlCtx.paramNode->name,
                             nxmlCtx.paramValue,
                             r2);
                //nxmlCtx.ret = r2;
            }

            if ((nxmlCtx.ret == VOS_RET_SUCCESS) && (nxmlCtx.loadMdm)
                    && (nxmlCtx.attr.accessBitMaskChange || nxmlCtx.attr.notificationChange))
            {

                vosLog_debug("set single param: %s attr notif=(%d)%d access=(%d)0x%x",
                             nxmlCtx.paramNode->name,
                             nxmlCtx.attr.notificationChange,
                             nxmlCtx.attr.notification,
                             nxmlCtx.attr.accessBitMaskChange,
                             nxmlCtx.attr.accessBitMask);

                r2 = mdm_setSingleParamNodeAttributes(nxmlCtx.paramNode, &(nxmlCtx.iidStack), &(nxmlCtx.attr), FALSE);
                if (r2 != VOS_RET_SUCCESS)
                {
                    vosLog_error("setSingle attr failed for %s %s notification=(%d)%d accessBitMask=(%d)0x%x ret=%d",
                                 nxmlCtx.paramNode->name,
                                 mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                                 nxmlCtx.attr.notificationChange,
                                 nxmlCtx.attr.notification,
                                 nxmlCtx.attr.accessBitMaskChange,
                                 nxmlCtx.attr.accessBitMask,
                                 r2);
                    //nxmlCtx.ret = r2;
                }
            }

            nxmlCtx.attr.accessBitMaskChange = 0;
            nxmlCtx.attr.notificationChange = 0;

            VOS_MEM_FREE_BUF_AND_NULL_PTR(nxmlCtx.paramValue);
            nxmlCtx.paramNode = NULL;
        }
    }
    else
    {
        /*
            * This must be the end tag of an object.
            */

        /* verify the end tag matches the start tag */
        if ((!util_strcmp(nxmlCtx.objNode->name, buf)) ||
                ((nxmlCtx.objNode == mdmShmCtx->rootObjNode) && (!util_strcmp(buf, CONFIG_FILE_TOP_NODE))))
        {
            /* this is the correct tag matching case, do nothing. */
        }
        else
        {
            vosLog_error("unexpected end tag, expected obj name %s got %s",
                         nxmlCtx.objNode->name, buf);
            nxmlCtx.ret = VOS_RET_INVALID_ARGUMENTS;
        }

        /*
         * As a special case check, there could be a snipet that looks
         * like this which is at the end of the config file:
         *       <someobject notification=2 accessList=tr69c,telnetd>
         *       </someobject>
         *     </someParentObject>
         *   </InternetGatewayDevice>
         * </DslCpeConfig>
         * (There is no transition to another object or a param.)
         * In which case, the next few lines of code will set the attribute for
         * that object.
         */
        if ((nxmlCtx.ret == VOS_RET_SUCCESS) &&
                (nxmlCtx.loadMdm) &&
                (nxmlCtx.attr.accessBitMaskChange || nxmlCtx.attr.notificationChange))
        {
            VOS_RET_E r2;

            vosLog_debug("set sub-tree %s %s attr notif=(%d)%d access=(%d)0x%x",
                         nxmlCtx.objNode->name,
                         mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                         nxmlCtx.attr.notificationChange,
                         nxmlCtx.attr.notification,
                         nxmlCtx.attr.accessBitMaskChange,
                         nxmlCtx.attr.accessBitMask);

            r2 = mdm_setSubTreeParamAttributes(nxmlCtx.objNode,
                                               &(nxmlCtx.iidStack),
                                               &(nxmlCtx.attr),
                                               FALSE);
            if (r2 != VOS_RET_SUCCESS)
            {
                vosLog_error("setSubTree attr failed for %s %s notification=(%d)%d accessBitMask=(%d)0x%x ret=%d",
                             nxmlCtx.objNode->name,
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)),
                             nxmlCtx.attr.notificationChange,
                             nxmlCtx.attr.notification,
                             nxmlCtx.attr.accessBitMaskChange,
                             nxmlCtx.attr.accessBitMask,
                             r2);
                nxmlCtx.ret = r2;
            }

            nxmlCtx.attr.accessBitMaskChange = 0;
            nxmlCtx.attr.notificationChange = 0;
        }

        /*
         * Set the MdmObject into the MDM.  We need to do this before we pop
         * any instance id's off of the iidStack.  Only the first end object
         * tag will have an MdmObject.  All other higher level objects were
         * already set when the parsing transitioned down to the lower level
         * begin tag.
         */
        if (nxmlCtx.ret == VOS_RET_SUCCESS)
        {
            if (nxmlCtx.mdmObj != NULL && nxmlCtx.nextInstanceNode == FALSE)
            {
                vosLog_debug("setting obj %s %s",
                             mdm_oidToGenericPath(*((MdmObjectId *)nxmlCtx.mdmObj)),
                             mdm_dumpIidStack(&(nxmlCtx.iidStack)));
                if (nxmlCtx.loadMdm)
                {
                    VOS_RET_E r2;

                    r2 = mdm_setObject(&(nxmlCtx.mdmObj), &(nxmlCtx.iidStack), FALSE);
                    if (r2 != VOS_RET_SUCCESS)
                    {
                        vosLog_error("setObject failed, %d", r2);
                        mdm_freeObject(&(nxmlCtx.mdmObj));
                        nxmlCtx.ret = r2;
                    }
                }
                else
                {
                    mdm_freeObject(&(nxmlCtx.mdmObj));
                }
            }
        }


        if (nxmlCtx.objNode->parent != NULL)
        {
            /* normal end of object processing. */

            if (IS_INDIRECT2(nxmlCtx.objNode))
            {
                POP_INSTANCE_ID(&(nxmlCtx.iidStack));
            }

            nxmlCtx.objNode = nxmlCtx.objNode->parent;
            vosLog_debug("pop up to %s %s",
                         mdm_oidToGenericPath(nxmlCtx.objNode->oid),
                         mdm_dumpIidStack(&(nxmlCtx.iidStack)));
        }

        /*
         * Remember the fact we have seen the end tag for this object.
         * This is needed for the special case where we have an object
         * called WEPKey and a parameter called WEPKey and we need to
         * know whether we are about to load the WEPKey object or the
         * WEPKey parameter.  WEPKey object can only be loaded when
         * the end tag of the previous object has been seen.
         */
        nxmlCtx.gotCurrObjEndTag = TRUE;
    }
}


static void recoverFactoryParam(UBOOL8 isRecover)
{
    UBOOL8 restoreMode = FALSE;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    char buf[BUFLEN_256] = {0};
    char wlanSsid[BUFLEN_64] = {0};
    char wlanSsid2[BUFLEN_64] = {0};
    char wlanSsid3[BUFLEN_64] = {0};
    char wlanSsid4[BUFLEN_64] = {0};
    FILE *fp = NULL;
    int mode = 0;
    UBOOL8 setSSID2Flag = FALSE;
    UBOOL8 setSSID3Flag = FALSE;
    UBOOL8 setSSID4Flag = FALSE;

    ret = HAL_sysGetRestoreMode(&restoreMode);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Get restore mode error, ret=%u", ret);
    }

    if (!isRecover && !restoreMode)
    {
        vosLog_notice("isRecover=%d, restoreMode=%d", isRecover, restoreMode);
        return;
    }

    if ((fp = fopen("/var/config/restoremode", "r")) != NULL)
    {
        fscanf(fp, "%d", &mode);
        fclose(fp);
        UTIL_doSystemAction("delete restore mode file", "rm -f /var/config/restoremode");
    }
    else
    {
        printf("open MDM_LOCAL_RESTORE_FILE failed");
    }

    if (SF_FEATURE_SUPPORT_WLAN)
    {
        if (SF_FEATURE_CUSTOMER_JINQIANMAO || SF_FEATURE_CUSTOMER_DONGYAN || SF_FEATURE_CUSTOMER_JSWX
                || (SF_FEATURE_CUSTOMER_KINGTYPE && SF_FEATURE_LOCATION_SHAANXI) || SF_FEATURE_CUSTOMER_H3C ||
                SF_FEATURE_CUSTOMER_JISHIHUITONG)
        {
            if (restoreMode)
            {
                UINT8 ucTemp[6] = {0};
                char strAddr[7] = {0};

                ret = HAL_sysGetMacAddr(ucTemp);
                if (VOS_RET_SUCCESS == ret)
                {
                    UTIL_SNPRINTF(strAddr, sizeof(strAddr), "%2.2X%2.2X%2.2X", ucTemp[3], ucTemp[4], ucTemp[5]);

                    if (SF_FEATURE_CUSTOMER_JINQIANMAO)
                    {
                        if (SF_FEATURE_LOCATION_HAINAN)
                        {
                            UTIL_STRNCPY(wlanSsid, "Hi-Cable-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                        }
                        else if (SF_FEATURE_LOCATION_SICHUAN)
                        {
                            UTIL_SNPRINTF(strAddr, sizeof(strAddr), "%2.2X%2.2X", ucTemp[4], ucTemp[5]);
                            UTIL_STRNCPY(wlanSsid, "JQM-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid, "-A", sizeof(wlanSsid));
                            UTIL_STRNCPY(wlanSsid2, "JQM-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid2, strAddr, sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid2, "-B", sizeof(wlanSsid));
                            UTIL_STRNCPY(wlanSsid3, "JQM-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid3, strAddr, sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid3, "-C", sizeof(wlanSsid));
                            UTIL_STRNCPY(wlanSsid4, "JQM-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid4, strAddr, sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid4, "-D", sizeof(wlanSsid));
                        }
                        else
                        {
                            UTIL_STRNCPY(wlanSsid, "JQM-", sizeof(wlanSsid));
                            UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                        }
                    }
                    else if (SF_FEATURE_CUSTOMER_DONGYAN)
                    {
                        char strAddr2[5] = {0};
                        UTIL_STRNCPY(wlanSsid, "CMCC_", sizeof(wlanSsid));
                        UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));

                        UTIL_SNPRINTF(strAddr2, sizeof(strAddr2), "%2.2X%2.2X", ucTemp[4], ucTemp[5]);
                        UTIL_STRNCPY(wlanSsid2, "GCable_", sizeof(wlanSsid2));
                        UTIL_STRNCAT(wlanSsid2, strAddr2, sizeof(wlanSsid2));

                    }
                    else if (SF_FEATURE_CUSTOMER_JSWX)
                    {
                        UTIL_SNPRINTF(wlanSsid, sizeof(wlanSsid) - 1, "user%s", strAddr);
                    }
                    else if (SF_FEATURE_CUSTOMER_KINGTYPE && SF_FEATURE_LOCATION_SHAANXI)
                    {
                        UTIL_STRNCPY(wlanSsid, "sxbctv-", sizeof(wlanSsid));
                        UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                    }
                    else if (SF_FEATURE_CUSTOMER_H3C)
                    {
                        UTIL_STRNCPY(wlanSsid, "H3C_", sizeof(wlanSsid));
                        UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                    }
                    else if (SF_FEATURE_WEB_STYLE_CN2 && SF_FEATURE_CUSTOMER_JISHIHUITONG)
                    {
                        UTIL_STRNCPY(wlanSsid, "JSM-", sizeof(wlanSsid));
                        UTIL_STRNCAT(wlanSsid, strAddr, sizeof(wlanSsid));
                    }
                }
                else
                {
                    vosLog_error("HAL_sysGetMacAddr fail, ret = %d", ret);
                    return;
                }
            }
        }
        else
        {
            memset(buf, 0, sizeof(buf));
            HAL_sysGetSsid(buf, sizeof(buf));
            /* set SSID */
            if (buf[0] != '\0')
            {
                char *ptr = NULL;

                UTIL_STRNCPY(wlanSsid, buf, sizeof(wlanSsid));

                if (mode != 1)
                {
                    if (SF_FEATURE_LOCATION_CHONGQING && SF_FEATURE_ISP_CT)
                    {
                        UTIL_STRNCPY(wlanSsid2, "IPTV-", sizeof(wlanSsid2));
                    }
                    else if (SF_FEATURE_LOCATION_JIANGXI && SF_FEATURE_ISP_CT)
                    {
                        UTIL_STRNCPY(wlanSsid2, "ChinaNet-iTV-", sizeof(wlanSsid2));
                    }
                    else if (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU)
                    {
                        //UTIL_STRNCPY(wlanSsid2, "VIDEOPHONE_", sizeof(wlanSsid2));
                        UTIL_STRNCPY(wlanSsid2, "GUEST_", sizeof(wlanSsid2));
                    }
                    else if (SF_FEATURE_ISP_CT)
                    {
                        UTIL_STRNCPY(wlanSsid2, "iTV-", sizeof(wlanSsid2));
                    }
                    else if ((SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU)
                    {
                        UTIL_STRNCPY(wlanSsid2, "CU_002_", sizeof(wlanSsid2));
                        UTIL_STRNCPY(wlanSsid3, "CU_003_", sizeof(wlanSsid3));
                        UTIL_STRNCPY(wlanSsid4, "CU_004_", sizeof(wlanSsid4));
                    }

                    if (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU)
                    {
                        UTIL_STRNCPY(wlanSsid3, "STB_", sizeof(wlanSsid3));
                        UTIL_STRNCPY(wlanSsid4, "chinaunicom", sizeof(wlanSsid4));
                    }

                    if (SF_FEATURE_LOCATION_HUBEI && SF_FEATURE_ISP_CT)
                    {
                        UTIL_STRNCPY(wlanSsid3, "ChinaNet-", sizeof(wlanSsid3));
                        UTIL_STRNCPY(wlanSsid4, "ChinaNet-", sizeof(wlanSsid4));
                    }

                    if ((SF_FEATURE_LOCATION_BEIJING || SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU)
                    {
                        ptr = strchr(wlanSsid, '_');
                    }
                    else if (SF_FEATURE_LOCATION_HAINAN && SF_FEATURE_ISP_CT)
                    {
                        UTIL_STRNCPY(wlanSsid2, wlanSsid, sizeof(wlanSsid2));
                        UTIL_STRNCAT(wlanSsid2, "-iTV", sizeof(wlanSsid2));
                    }
                    else if (SF_FEATURE_LOCATION_SHANDONG && SF_FEATURE_ISP_CU)
                    {
                        ptr = strchr(wlanSsid, '_');
                        UTIL_STRNCPY(wlanSsid2, "CU_iTV_", sizeof(wlanSsid2));
                    }
                    else if (SF_FEATURE_ISP_CT)
                    {
                        ptr = strchr(wlanSsid, '-');
                    }

                    if (ptr != NULL)
                    {
                        ptr = ptr + 1;
                        UTIL_STRNCAT(wlanSsid2, ptr, sizeof(wlanSsid2));
                        UTIL_STRNCAT(wlanSsid3, ptr, sizeof(wlanSsid3));

                        if ((SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU)
                        {
                            UTIL_STRNCAT(wlanSsid4, ptr, sizeof(wlanSsid4));
                        }

                        if (SF_FEATURE_LOCATION_HUBEI)
                        {
                            UTIL_STRNCAT(wlanSsid3, "-3", sizeof(wlanSsid3));
                            UTIL_STRNCAT(wlanSsid4, ptr, sizeof(wlanSsid4));
                            UTIL_STRNCAT(wlanSsid4, "-4", sizeof(wlanSsid4));
                        }
                    }
                }
            }
        }

        if (wlanSsid[0] != '\0')
        {
            LanWlanObject *wirelessObj = NULL;
            INIT_INSTANCE_ID_STACK(&iidStack);
            PUSH_INSTANCE_ID(&iidStack, 1);
            PUSH_INSTANCE_ID(&iidStack, 1);

            if (mdm_getObject(MDMOID_LAN_WLAN, &iidStack, (void **)&wirelessObj) == VOS_RET_SUCCESS)
            {
                VOS_MEM_REPLACE_STRING_FLAGS(wirelessObj->SSID, wlanSsid, mdmLibCtx.allocFlags);
                if ((ret = mdm_setObject((void **)&wirelessObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("Cant't set SSID1 ret = %d \n", ret);
                    mdm_freeObject((void **)&wirelessObj);
                }
            }
            else
            {
                vosLog_error("get MDMOID_LAN_WLAN(%d:%s) error", MDMOID_LAN_WLAN, mdm_dumpIidStack(&iidStack));
                return;
            }

            if (mode != 1)
            {
                if (wlanSsid2[0] != '\0')
                {
                    if (mdm_getNextObject(MDMOID_LAN_WLAN, &iidStack, (void **) &wirelessObj) == VOS_RET_SUCCESS)
                    {
                        if ((1 == wirelessObj->enable) || SF_FEATURE_LOCATION_HUBEI || SF_FEATURE_CUSTOMER_DONGYAN ||
                                SF_FEATURE_LOCATION_HUNAN || SF_FEATURE_LOCATION_SHAANXI
                                || (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU) || ((SF_FEATURE_LOCATION_SHANGHAI ||
                                        SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU)
                                || (SF_FEATURE_LOCATION_SICHUAN && SF_FEATURE_CUSTOMER_JINQIANMAO))
                        {
                            setSSID2Flag = TRUE;
                            VOS_MEM_REPLACE_STRING_FLAGS(wirelessObj->SSID, wlanSsid2, mdmLibCtx.allocFlags);
                            if ((ret = mdm_setObject((void **)&wirelessObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                            {
                                vosLog_error("Cant't set SSID2 ret = %d \n", ret);
                                mdm_freeObject((void **)&wirelessObj);
                            }
                        }
                    }
                }

                if (wlanSsid3[0] != '\0')
                {
                    if (mdm_getNextObject(MDMOID_LAN_WLAN, &iidStack, (void **) &wirelessObj) == VOS_RET_SUCCESS)
                    {
                        setSSID3Flag = TRUE;
                        if (1 == wirelessObj->enable || SF_FEATURE_LOCATION_HUBEI || ((SF_FEATURE_LOCATION_BEIJING ||
                                SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU)
                                || (SF_FEATURE_LOCATION_SICHUAN && SF_FEATURE_CUSTOMER_JINQIANMAO))
                        {
                            VOS_MEM_REPLACE_STRING_FLAGS(wirelessObj->SSID, wlanSsid3, mdmLibCtx.allocFlags);
                            if ((ret = mdm_setObject((void **)&wirelessObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                            {
                                vosLog_error("Cant't set SSID3 ret = %d \n", ret);
                                mdm_freeObject((void **)&wirelessObj);
                            }
                        }
                    }
                }

                if (wlanSsid4[0] != '\0')
                {
                    if (mdm_getNextObject(MDMOID_LAN_WLAN, &iidStack, (void **) &wirelessObj) == VOS_RET_SUCCESS)
                    {
                        setSSID4Flag = TRUE;
                        if (1 == wirelessObj->enable || SF_FEATURE_LOCATION_HUBEI || ((SF_FEATURE_LOCATION_BEIJING ||
                                SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN)  && SF_FEATURE_ISP_CU)
                                || (SF_FEATURE_LOCATION_SICHUAN && SF_FEATURE_CUSTOMER_JINQIANMAO))
                        {
                            VOS_MEM_REPLACE_STRING_FLAGS(wirelessObj->SSID, wlanSsid4, mdmLibCtx.allocFlags);
                            if ((ret = mdm_setObject((void **)&wirelessObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                            {
                                vosLog_error("Cant't set SSID4 ret = %d \n", ret);
                                mdm_freeObject((void **)&wirelessObj);
                            }
                        }
                    }
                }
            }
        }

        memset(buf, 0, sizeof(buf));
        HAL_sysGetWlanKey(buf, sizeof(buf));
        /* set wpaPsk */
        if (buf[0] != '\0')
        {
            LanWlanPreSharedKeyObject *wlanPreSharedKeyObj = NULL;
            INIT_INSTANCE_ID_STACK(&iidStack);
            PUSH_INSTANCE_ID(&iidStack, 1);
            PUSH_INSTANCE_ID(&iidStack, 1);
            PUSH_INSTANCE_ID(&iidStack, 1);

            if (mdm_getObject(MDMOID_LAN_WLAN_PRE_SHARED_KEY, &iidStack, (void **)&wlanPreSharedKeyObj) == VOS_RET_SUCCESS)
            {
                VOS_MEM_REPLACE_STRING_FLAGS(wlanPreSharedKeyObj->preSharedKey, buf, mdmLibCtx.allocFlags);

                if ((ret = mdm_setObject((void **)&wlanPreSharedKeyObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("Cant't set preSharedKey ret = %d \n", ret);
                    mdm_freeObject((void **)&wlanPreSharedKeyObj);
                }
            }
            else
            {
                vosLog_error("get MDMOID_LAN_WLAN_PRE_SHARED_KEY(%d:%s) error", MDMOID_LAN_WLAN_PRE_SHARED_KEY,
                             mdm_dumpIidStack(&iidStack));
                return;
            }

            if (mode != 1)
            {
                if (setSSID2Flag && !((SF_FEATURE_LOCATION_SHANGHAI || SF_FEATURE_LOCATION_HAINAN) && SF_FEATURE_ISP_CU))
                {
                    setSSID2Flag = FALSE;
                    if (mdm_getNextObject(MDMOID_LAN_WLAN_PRE_SHARED_KEY, &iidStack, (void **) &wlanPreSharedKeyObj) == VOS_RET_SUCCESS)
                    {
                        VOS_MEM_REPLACE_STRING_FLAGS(wlanPreSharedKeyObj->preSharedKey, buf, mdmLibCtx.allocFlags);

                        if ((ret = mdm_setObject((void **)&wlanPreSharedKeyObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                        {
                            vosLog_error("Cant't set preSharedKey ret = %d \n", ret);
                            mdm_freeObject((void **)&wlanPreSharedKeyObj);
                        }
                    }
                }

                if (setSSID3Flag && (SF_FEATURE_LOCATION_HUBEI || (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU)))
                {
                    setSSID3Flag = FALSE;
                    if (mdm_getNextObject(MDMOID_LAN_WLAN_PRE_SHARED_KEY, &iidStack, (void **) &wlanPreSharedKeyObj) == VOS_RET_SUCCESS)
                    {
                        VOS_MEM_REPLACE_STRING_FLAGS(wlanPreSharedKeyObj->preSharedKey, buf, mdmLibCtx.allocFlags);

                        if ((ret = mdm_setObject((void **)&wlanPreSharedKeyObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                        {
                            vosLog_error("Cant't set preSharedKey ret = %d \n", ret);
                            mdm_freeObject((void **)&wlanPreSharedKeyObj);
                        }
                    }
                }

                if (setSSID4Flag && (SF_FEATURE_LOCATION_HUBEI || (SF_FEATURE_LOCATION_BEIJING && SF_FEATURE_ISP_CU)))
                {
                    setSSID4Flag = FALSE;
                    if (mdm_getNextObject(MDMOID_LAN_WLAN_PRE_SHARED_KEY, &iidStack, (void **) &wlanPreSharedKeyObj) == VOS_RET_SUCCESS)
                    {
                        VOS_MEM_REPLACE_STRING_FLAGS(wlanPreSharedKeyObj->preSharedKey, buf, mdmLibCtx.allocFlags);

                        if ((ret = mdm_setObject((void **)&wlanPreSharedKeyObj, &iidStack, FALSE)) != VOS_RET_SUCCESS)
                        {
                            vosLog_error("Cant't set preSharedKey ret = %d \n", ret);
                            mdm_freeObject((void **)&wlanPreSharedKeyObj);
                        }
                    }
                }
            }
        }
    }

    memset(buf, 0, sizeof(buf));
    HAL_sysGetUserPwd(buf, sizeof(buf));
    /* set user Password */
    if (buf[0] != '\0')
    {
        LoginCfgObject *loginCfg = NULL;
        ServiceManageObject *serviceFtpObj = NULL;
        LanServiceManageObject *lanServiceFtpObject = NULL;
        char userName[BUFLEN_16] = {0};
        char userPassword[BUFLEN_16] = {0};

        INIT_INSTANCE_ID_STACK(&iidStack);
        ret = cmcObj_get(MDMOID_LOGIN_CFG, &iidStack, 0, (void **)&loginCfg);
        if (ret == VOS_RET_SUCCESS)
        {
            UTIL_STRNCPY(userName, loginCfg->userUserName, sizeof(userName));
            if (SF_FEATURE_WEB_STYLE_CN2)
            {
                UTIL_STRNCPY(userPassword, loginCfg->userPassword, sizeof(userName));
            }
            else
            {
                VOS_MEM_REPLACE_STRING(loginCfg->userPassword, buf);
            }

            if (SF_FEATURE_UPLINK_TYPE_EOC)
            {
                memset(buf, 0, sizeof(buf));
                HAL_sysGetUserName(buf, sizeof(buf));
                if (buf[0] != '\0')
                {
                    VOS_MEM_REPLACE_STRING(loginCfg->userUserName, buf);
                }
            }

            cmcObj_set(loginCfg, &iidStack);
            cmcObj_free((void **)&loginCfg);
        }

        if (SF_FEATURE_SUPPORT_PLUGIN)
        {
            INIT_INSTANCE_ID_STACK(&iidStack);
            if (VOS_RET_SUCCESS == (ret = cmcObj_get(MDMOID_SERVICE_MANAGE, &iidStack, 0, (void **)&serviceFtpObj)))
            {
                if ('\0' != userName[0])
                {
                    VOS_MEM_REPLACE_STRING(serviceFtpObj->ftpUserName, userName);
                }
                else
                {
                    VOS_MEM_REPLACE_STRING(serviceFtpObj->ftpUserName, "useradmin");
                }

                VOS_MEM_REPLACE_STRING(serviceFtpObj->ftpPassword, buf);
                vosLog_debug("ftpUserName=%s, ftpPassword=%s", serviceFtpObj->ftpUserName, serviceFtpObj->ftpPassword);

                if ((ret = cmcObj_set(serviceFtpObj, &iidStack)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("set of service manage ftp config failed");
                }
                cmcObj_free((void **)&serviceFtpObj);
            }

            INIT_INSTANCE_ID_STACK(&iidStack);
            if (VOS_RET_SUCCESS == (ret = cmcObj_get(MDMOID_LAN_SERVICE_MANAGE, &iidStack, 0, (void **) &lanServiceFtpObject)))
            {
                if ('\0' != userName[0])
                {
                    VOS_MEM_REPLACE_STRING(lanServiceFtpObject->ftpUserName, userName);
                }
                else
                {
                    VOS_MEM_REPLACE_STRING(lanServiceFtpObject->ftpUserName, "useradmin");
                }

                VOS_MEM_REPLACE_STRING(lanServiceFtpObject->ftpPassword, buf);

                if ((ret = cmcObj_set(lanServiceFtpObject, &iidStack)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("set of lan service manage ftp config failed");
                }
                cmcObj_free((void **) &lanServiceFtpObject);
            }
        }
    }

    if (SF_FEATURE_LOCATION_FUJIAN)
    {
        if (buf[0] != '\0')
        {
            _LanServiceManageObject *lanServiceManageObject = NULL;

            INIT_INSTANCE_ID_STACK(&iidStack);
            ret = cmcObj_get(MDMOID_LAN_SERVICE_MANAGE, &iidStack, 0, (void **)&lanServiceManageObject);

            if (VOS_RET_SUCCESS == ret)
            {
                VOS_MEM_REPLACE_STRING(lanServiceManageObject->ftpPassword, buf);
                VOS_MEM_REPLACE_STRING(lanServiceManageObject->telnetPassword, buf);

                cmcObj_set(lanServiceManageObject, &iidStack);
                cmcObj_free((void **)&lanServiceManageObject);
            }
        }
    }

    if (restoreMode)
    {
        restoreMode = FALSE;
        HAL_sysSetRestoreMode(restoreMode);
    }
}


/** Nanoxml does not pass context to callback func, so we have to use a global var */
struct nanoxml_context nxmlCtx;

struct nanoxml_context
{
    UBOOL8 loadMdm;            /**< Insert results of xml parsing into MDM */
    UBOOL8 topNodeFound;       /**< Have we seen the DslCpeConfig node yet? */
    UBOOL8 versionFound;       /**< Have we seen the version attribute yet? */
    UBOOL8 nextInstanceNode;   /**< currently processing a special nextInstance node */
    UBOOL8 gotCurrObjEndTag;   /**< we have seen the end tag for the current object */

    UINT32 versionMajor;     /**< Config file version major */
    UINT32 versionMinor;     /**< Config file version minor.  Not used, but feels weird if not provided. */

    MdmObjectNode *objNode;  /**< Current objNode we are processing */
    InstanceIdStack iidStack;/**< IidStack of the mdmObj we are building. */
    void *mdmObj;            /**< Current mdmObj we are building */
    MdmParamNode *paramNode; /**< Current paramNode we are processing */
    char *paramValue;         /**< Value of current param in string format. */
    MdmNodeAttributes attr;  /**< MdmNodeAttribute */
    enum mdm_config_attribute currXmlAttr;/**< XML attribute we are currently processing */
    VOS_RET_E ret;              /**< Any errors detected during callback. */
};


typedef enum
{
    VOS_RET_SUCCESS              = 0,     /**<Success. */
    VOS_RET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
    VOS_RET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
    VOS_RET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
    VOS_RET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
    VOS_RET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
    VOS_RET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
    VOS_RET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
    VOS_RET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
    VOS_RET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
    VOS_RET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
    VOS_RET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
    VOS_RET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
    VOS_RET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
    VOS_RET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
    VOS_RET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
    VOS_RET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
    VOS_RET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
    VOS_RET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
    VOS_RET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
    VOS_RET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
    VOS_RET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
    VOS_RET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
    VOS_RET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
    VOS_RET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

    VOS_RET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
    VOS_RET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
    VOS_RET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
    VOS_RET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
    VOS_RET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
    VOS_RET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
    VOS_RET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
    VOS_RET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */

    VOS_RET_DU_UPDATE_VERSION_EXISTED = 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
    VOS_RET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
    VOS_RET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
    VOS_RET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
    VOS_RET_FAIL_REBOOT_REQUIRED = 9803,  /**<Config failed, and now system is in a bad state requiring reboot. */
    VOS_RET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
    VOS_RET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
    VOS_RET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
    VOS_RET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
    VOS_RET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
    VOS_RET_TIMED_OUT = 9809,     /**<Operation timed out. */
    VOS_RET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
    VOS_RET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
    VOS_RET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
    VOS_RET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
    VOS_RET_OPEN_FILE_ERROR = 9818,     /**< open file error */
    VOS_RET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
    VOS_RET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
    VOS_RET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
    VOS_RET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */

    VOS_RET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
    VOS_RET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
    VOS_RET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
    VOS_RET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
    VOS_RET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
    VOS_RET_RESOURCE_NOT_CONFIGURED = 9855, /**< requested resource is not configured/found */
    VOS_RET_CARD_STATUS_FAILED = 9856,
    VOS_RET_LAN_SPEED_FAILED = 9857,
    VOS_RET_FILE_INVALID = 9858,
    VOS_RET_CONFIG_GET_FAILED = 9859,
    VOS_RET_WAN_CONNTYPE_NOT_IPROUTE = 9860,
    VOS_RET_GET_ROUTE_WAN_FAILED = 9861,
    VOS_RET_PLUGIN_NOT_EXIST  = 9862,
    VOS_RET_INVALID_TUNNEL_NAME = 9863,
    VOS_RET_INVALID_TUNNEL_USER_ID = 9864,
    VOS_RET_SOCK_WRITE_ERR_EAGAIN = 9865,
} VOS_RET_E;



int xmlOpen(nxml_t *handle, const nxml_settings *settings)
{
    if ((*handle = (nxml_t)VOS_MALLOC_FLAGS(sizeof(**handle), ALLOC_ZEROIZE)) != NULL)
    {
        (*handle)->settings = *settings;
        (*handle)->namecachesize = 0;
        (*handle)->treelevel = 0;
        (*handle)->skipwhitespace = 1;
        (*handle)->state = state_start;
        return 1;
    }
    return 0;
}

void xmlClose(nxml_t handle)
{
    VOS_FREE(handle);
}

int xmlWrite(nxml_t handle, char *data, unsigned len, char **endp)
{
    int treeEnd = 0;
    const char *enddata = data + len;

    vosLog_error(" ---- nxml_write(): len %d", len);

    if (NULL == data)
    {
        vosLog_error("NULL == data");
        return 0;
    }

    if (handle->state == state_start)
    {
        char *p;
        vosLog_error(" ---- skip <?xml header ");
        p = (char *) strskip(data, WHITESPACE);
        if (!strncmp(p, "<?", 2))
        {
            p = strstr(p, "?>");
            if (p)
                data = p + 2;
        }
        handle->state = state_begin_tag;
    }

    while ((data < enddata) && (!treeEnd))
    {
        char *s; /* temp value for capturing search results */

        /* skip whitespace */
        if (handle->skipwhitespace)
        {
            s = (char *) strskip(data, WHITESPACE);
            if (s != data)
            {
                data = s;
                continue;
            }
        }

        //        vosLog_debug(" ---- state %d, %c, %d", handle->state, *data, handle->namecachesize);

        switch (handle->state)
        {
            case state_begin_tag:
                s = strchr(data, '<');
                if (!s)
                {
                    /* it's all data */
                    if (handle->settings.data)
                        (*handle->settings.data)(handle, data, enddata - data, 1/*maybe more data*/);
                    *endp = data;
                    return 0;
                }
                else if ((data != s) && handle->settings.data)
                {
                    /* we have some data, then a tag */
                    (*handle->settings.data)(handle, data, s - data, 0/*end of data*/);
                }
                /* skip over the tag begin and process the tag name */
                data = s + 1;
                handle->state = state_tag_name;
                handle->namecachesize = 0;
                ++(handle->treelevel);
                break;

            case state_finish_tag:
                /* we don't care about anything but the end of a tag */
                s = strchr(data, '>');
                if (!s)
                {
                    *endp = data;
                    return 0;
                }

                /* we found it, so start looking for the next tag */
                data = s + 1;
                handle->state = state_begin_tag;
                if (--(handle->treelevel) <= 0)
                    treeEnd = 1;
                break;

            case state_end_tag_name:
                s = strpbrk(data, WHITESPACE ">");
                if (!s)
                {
                    /* it's all name, and we're not done */
                    nxml_add_namecache(handle, data, enddata - data, NULL, NULL);
                    handle->skipwhitespace = 0;
                    *endp = data;
                    return 0;
                }
                else
                {
                    char *name;
                    unsigned length;
                    nxml_add_namecache(handle, data, s - data, &name, &length);

                    /*if (strncmp(name,"psitree",len) == 0)
                      psiEnd = 1;*/

                    (*handle->settings.tag_end)(handle, name, length);
                    handle->state = state_finish_tag;
                    data = s;
                    if (--(handle->treelevel) <= 0)
                        treeEnd = 1;
                }
                break;

            case state_tag_name:
            case state_attr_name:
                if (*data == '/')
                {
                    /* this tag is done */
                    if (handle->state == state_tag_name && !handle->namecachesize)
                    {
                        /* we can still parse the end tag name so that the uppperlevel app
                        can validate if it cares */
                        handle->state = state_end_tag_name;
                        data++;
                        break;
                    }
                    else if (handle->state == state_attr_name)
                    {
                        /* we had an attribute, so this tag is just done */
                        (*handle->settings.tag_end)(handle, handle->namecache, handle->namecachesize);
                        handle->state = state_finish_tag;
                        data++;
                        break;
                    }
                }
                else if (*data == '>')
                {
                    handle->state = state_begin_tag;
                    data++;
                    break;
                }

                /* TODO: is = a valid in a tag? I don't think so. */
                s = (char *)strpbrk(data, WHITESPACE "=/>");
                if (!s)
                {
                    /* it's all name, and we're not done */
                    nxml_add_namecache(handle, data, enddata - data, NULL, NULL);
                    handle->skipwhitespace = 0;
                    *endp = data;
                    return 0;
                }
                else
                {
                    /* we have the entire name */
                    char *name;
                    unsigned length;
                    nxml_add_namecache(handle, data, s - data, &name, &length);

                    if (handle->state == state_tag_name)
                    {
                        (*handle->settings.tag_begin)(handle, name, length);
                        handle->state = state_attr_name;
                    }
                    else
                    {
                        (*handle->settings.attribute_begin)(handle, name, length);
                        handle->state = state_attr_value_equals;
                    }
                    handle->namecachesize = 0;
                    data = s;
                }
                break;

            case state_attr_value:
                s = (char *)strchr(data, '"');
                if (!s)
                {
                    /* it's all attribute_value, and we're not done */
                    (*handle->settings.attribute_value)(handle, data, enddata - data, 1);
                    handle->skipwhitespace = 0;
                    *endp = data;
                    return 0;
                }
                else
                {
                    /* we have some value data, then a tag */
                    (*handle->settings.attribute_value)(handle, data, s - data, 0);
                }
                /* skip over the quote and look for more attributes */
                data = s + 1;
                handle->state = state_attr_name;
                handle->namecachesize = 0;
                break;


            case state_attr_value_equals:
                if (*data == '>')
                {
                    handle->state = state_begin_tag;
                    data++;
                }
                else if (*data == '=')
                {
                    handle->state = state_attr_value_quote;
                    data++;
                }
                else
                    handle->state = state_attr_name;
                break;

            case state_attr_value_quote:
                if (*data == '"')
                    data++;
                handle->state = state_attr_value;
                break;
            default:
                break;
        }
        handle->skipwhitespace = 1;
    }  /* while () ... */
    *endp = data;
    return treeEnd ? 1 : 0;
}


VOS_RET_E mdm_validateConfigBuf(const char *buf, UINT32 len)
{
    nxml_t nxmlHandle;
    nxml_settings nxmlSettings;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char *endp = NULL;
    SINT32 rc = 0;

    /*
    * When reading from configflash that has been zeroized,
    * we get length of 1.  Way too short to hold valid config file.
    */
    if (buf == NULL || len <= 1)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

    /* initialize our callback context state */
    memset(&nxmlCtx, 0, sizeof(nxmlCtx));
    nxmlCtx.loadMdm = FALSE;
    nxmlCtx.currXmlAttr = MDM_CONFIG_XMLATTR_NONE;
    nxmlCtx.ret = VOS_RET_SUCCESS;

    /* set callback function handlers */
    nxmlSettings.tag_begin = mdm_tagBeginCallbackFunc;
    nxmlSettings.attribute_begin = mdm_attrBeginCallbackFunc;
    nxmlSettings.attribute_value = mdm_attrValueCallbackFunc;
    nxmlSettings.data = mdm_dataCallbackFunc;
    nxmlSettings.tag_end = mdm_tagEndCallbackFunc;

    rc = xmlOpen(&nxmlHandle, &nxmlSettings);
    if (0 == rc)
    {
        vosLog_error("xmlOpen failed");
        return VOS_RET_INTERNAL_ERROR;
    }

    /* push our data into the nanoxml parser, it will then call our callback funcs. */
    rc = xmlWrite(nxmlHandle, (char *)buf, len, &endp);
    if (0 == rc)
    {
        vosLog_error("nanoxml parser returned error.");
        ret = VOS_RET_INVALID_ARGUMENTS;
    }
    else if (nxmlCtx.ret != VOS_RET_SUCCESS)
    {
        vosLog_notice("nxmlCtx.ret = %d", nxmlCtx.ret);
        ret = nxmlCtx.ret;
    }
    else if (!isAllWhiteSpace(endp, buf + len - endp))
    {
        /* nanoxml endp points at one past the last processed char */
        vosLog_error("not all data processed, buf %p (len 0x%x) "
                     "endp at %p, buf+len-endp=%d",
                     buf, len, endp, buf + len - endp);
        ret = VOS_RET_INVALID_ARGUMENTS;
    }
    else if (!nxmlCtx.topNodeFound || !nxmlCtx.versionFound)
    {
        vosLog_error("did not find top node and/or version");
        ret = nxmlCtx.ret;
    }

    xmlClose(nxmlHandle);

    return ret;
}

VOS_RET_E HAL_flashFileRead(char *fileName, char *buf, UINT32 *bufLen)
{
#ifdef DESKTOP_LINUX
    return VOS_RET_SUCCESS;
#else /* DESKTOP_LINUX */
    VOS_RET_E ret = VOS_RET_SUCCESS;
    SINT32 fd = -1;
    UINT32 length = 0;

    if (NULL == fileName || NULL == buf || NULL == bufLen)
    {
        vosLog_error("Invalid args!\n");
        endret(VOS_RET_INVALID_ARGUMENTS);
    }

    fd = open(fileName, O_CREAT | O_RDWR, S_IRWXU);
    if (-1 == fd)
    {
        vosLog_error("Error: open %s failed, errno:%d,%s\n", fileName, errno, strerror(errno));
        return VOS_RET_IMAGE_FLASH_FAILED;
    }

    lseek(fd, 0, SEEK_SET);
    length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if (length <= 0)
    {
        vosLog_error("%s don't has valid data, errno:%d,%s!\n", fileName, errno, strerror(errno));
        return VOS_RET_IMAGE_FLASH_FAILED;
    }

    if (*bufLen > 0 && *bufLen < length)
    {
        /* prevent mem overflow, trunc buf length when read data into buf */
        length = *bufLen;
    }

    *bufLen = read(fd, (char *)buf, length);
    if (*bufLen != length)
    {
        vosLog_error("read %s file error, errno:%d,%s\n", fileName, errno, strerror(errno));
        return VOS_RET_IMAGE_FLASH_FAILED;
    }

end:

    if (-1 != fd)
    {
        close(fd);
    }

    return ret;
#endif
}


int main()
{
    char *fileName = "userDefaultconf";
    char *buf = NULL;
    UINT32 *bufLen;
    ret = HAL_flashFileRead(fileName, buf, bufLen);
    if (VOS_RET_SUCCESS == ret && buflen > 0)
    {
        vosLog_error("zheng ret = %d", ret);
        //ret = mdm_validateConfigBuf(buf, buflen);
        vosLog_error("zheng ret = %d", ret);
        if (ret != VOS_RET_SUCCESS)
        {
            vosLog_error("CONFIG is unrecognized or invalid.");
        }
        else
        {
            vosLog_print("upload config from CONFIG\n");
        }
    }

    return 0;
}

