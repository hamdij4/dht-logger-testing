package main

import (
	"hamdij4/server/utils"
	"hamdij4/server/consts"
	"hamdij4/server/controllers"
	"net/http"
	"time"
)

func main(){

	utils.PrettyLog(consts.LOG_TYPE_INFO, "System starting")

	controllers.Uptime = time.Now()

	http.HandleFunc("/", controllers.HandleOnlineCheck)
	http.HandleFunc("/online", controllers.HandleOnlineCheck)
	http.HandleFunc("/client/data", controllers.HandleClientGET)
	http.HandleFunc("/data", controllers.HandleDataRX)

	err := http.ListenAndServe(":8000", nil)
	if err != nil {
		utils.PrettyLog(consts.LOG_TYPE_ERROR, "Failed to start server")
		return
	}

}
