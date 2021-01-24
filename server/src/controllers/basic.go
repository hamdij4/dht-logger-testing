package controllers

import (
	"fmt"
	"net/http"
	"github.com/hamdij4/dht-logger-testing/src/main/utils"
	"github.com/hamdij4/dht-logger-testing/src/main/consts"
)

// GET /online
func HandleOnlineCheck(w http.ResponseWriter, req *http.Request){
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Server online")
	fmt.Println(req)
	return
}
