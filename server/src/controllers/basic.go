package controllers

import (
	"fmt"
	"net/http"
	"hamdij4/server/utils"
	"hamdij4/server/consts"
)

// GET /online
func HandleOnlineCheck(w http.ResponseWriter, req *http.Request){
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Server online")
	fmt.Println(req)
	fmt.Fprintf(w, "Status - OK\n")
	return
}
func IsOk(w http.ResponseWriter, req *http.Request){
	fmt.Println(req)
	fmt.Fprintf(w, "hello\n")
	return
}
