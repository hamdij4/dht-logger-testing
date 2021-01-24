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

	controllers.Start = time.Now()

	http.HandleFunc("/online", controllers.HandleOnlineCheck)
	http.HandleFunc("/client/data", controllers.HandleClientGET)
	http.HandleFunc("/data", controllers.HandleDataRX)
	buildHandler := http.FileServer(http.Dir("../../client/dht-client/build"))
	http.Handle("/", buildHandler)
	http.HandleFunc("/test", func(writer http.ResponseWriter, request *http.Request) {
		http.ServeFile(writer, request, "../../client/dht-client/build/index.html")
	})

	err := http.ListenAndServe(":8000", nil)
	if err != nil {
		utils.PrettyLog(consts.LOG_TYPE_ERROR, "Failed to start server")
		return
	}

}
