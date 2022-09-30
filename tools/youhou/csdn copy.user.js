// ==UserScript==
// @name         csdn copy
// @namespace    http://tampermonkey.net/
// @version      0.1
// @description  try to take over the world!
// @author       You
// @match        https://*.blog.csdn.net/*
// @icon         data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==
// @grant        none
// ==/UserScript==

(function() {
    $("#content_views pre").css("user-select","text");
    $("#content_views pre code").css("user-select","text");
})();

(function () {
  // Your code here...
  // 使用 Jquery 选择器，也可以直接使用原生的 JS
  $("#article_content").removeAttr("style");
  $(".hide-article-box").remove();
})();