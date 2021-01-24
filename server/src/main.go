package main

import (
	"github.com/hamdij4/dht-logger-testing/src/main/utils"
	"github.com/hamdij4/dht-logger-testing/src/main/consts"
	"github.com/hamdij4/dht-logger-testing/src/main/controllers"
	"net/http"
)

func main(){

	utils.PrettyLog(consts.LOG_TYPE_INFO, "System starting")

	http.HandleFunc("/online", controllers.HandleOnlineCheck)

	err := http.ListenAndServe(":8000", nil)
	if err != nil {
		utils.PrettyLog(consts.LOG_TYPE_ERROR, "Failed to start server")
		return
	}

}
