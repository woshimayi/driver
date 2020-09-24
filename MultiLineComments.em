macro MultiLineComment()
{
    hwnd = GetCurrentWnd()
    selection = GetWndSel(hwnd)
    LnFirst = GetWndSelLnFirst(hwnd)      //取首行行号
    LnLast = GetWndSelLnLast(hwnd)      //取末行行号
    hbuf = GetCurrentBuf()
 
    if(GetBufLine(hbuf, 0) == "//magic-number:tph85666031"){
        stop
    }
 
    Ln = Lnfirst
    buf = GetBufLine(hbuf, Ln)
    len = strlen(buf)
 
    while(Ln <= Lnlast) 
    {
        buf = GetBufLine(hbuf, Ln)  //取Ln对应的行
        if(buf == "")
        {                    //跳过空行
            Ln = Ln + 1
            continue
        }
 
        if(StrMid(buf, 0, 1) == "/")
        {       //需要取消注释,防止只有单字符的行
            if(StrMid(buf, 1, 2) == "*")
            {
                PutBufLine(hbuf, Ln, StrMid(buf, 2, Strlen(buf)))
            }
        }
 		
        if(StrMid(buf,0,1) != "/")
        {          //需要添加注释
            PutBufLine(hbuf, Ln, Cat("/*", buf))
        }
        
        Ln = Ln + 1
    }
    SetWndSel(hwnd, selection)

	// 添加末尾注释
	Ln = Lnfirst
    buf = GetBufLine(hbuf, Ln)
    len = strlen(buf)
 
    while(Ln <= Lnlast) 
    {
        buf = GetBufLine(hbuf, Ln)  //取Ln对应的行
        if(buf == "")
        {                    //跳过空行
            Ln = Ln + 1
            continue
        }
 
        if(StrMid(buf, Strlen(buf)-1, Strlen(buf)) == "/")
        {       //需要取消注释,防止只有单字符的行
            if(StrMid(buf, Strlen(buf)-2, Strlen(buf)-1) == "*")
            {
                PutBufLine(hbuf, Ln, StrMid(buf, 0, Strlen(buf)-2))
            }
        }
 		
        if(StrMid(buf,Strlen(buf)-1,Strlen(buf)) != "/")
        {          //需要添加注释
            PutBufLine(hbuf, Ln, Cat(buf, "*/"))
        }
        
        Ln = Ln + 1
    }
    SetWndSel(hwnd, selection)
    
}


macro AddMacroComment()
{
    hwnd=GetCurrentWnd()  
    sel=GetWndSel(hwnd)  
    lnFirst=GetWndSelLnFirst(hwnd)  
    lnLast=GetWndSelLnLast(hwnd)  
    hbuf=GetCurrentBuf()  

    if(LnFirst == 0) {  
            szIfStart = ""  
    }else{  
            szIfStart = GetBufLine(hbuf, LnFirst - 1)  
    }  
    szIfEnd = GetBufLine(hbuf, lnLast + 1)  
    if(szIfStart == "#if 0" && szIfEnd == "#endif") {  
            DelBufLine(hbuf, lnLast + 1)  
            DelBufLine(hbuf, lnFirst - 1)  
            sel.lnFirst = sel.lnFirst - 1  
            sel.lnLast = sel.lnLast - 1
    }else{  
            InsBufLine(hbuf, lnFirst, "#if 0")  
            InsBufLine(hbuf, lnLast + 2, "#endif")  
            sel.lnFirst = sel.lnFirst + 1  
            sel.lnLast = sel.lnLast + 1  
    }  

    SetWndSel( hwnd, sel )  
}