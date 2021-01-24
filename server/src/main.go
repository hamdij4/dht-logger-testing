package main

import (
	"hamdij4/server/utils"
	"hamdij4/server/consts"
	"hamdij4/server/controllers"
	"net/http"
)

func main(){

	utils.PrettyLog(consts.LOG_TYPE_INFO, "System starting")

	http.HandleFunc("/", controllers.IsOk)
	http.HandleFunc("/online", controllers.HandleOnlineCheck)

	err := http.ListenAndServe(":8000", nil)
	if err != nil {
		utils.PrettyLog(consts.LOG_TYPE_ERROR, "Failed to start server")
		return
	}

}
