package main

import (
	"encoding/json"
	"hamdij4/server/consts"
	"hamdij4/server/controllers"
	"hamdij4/server/models"
	"hamdij4/server/utils"
	"io/ioutil"
	"net/http"
	"strconv"
	"time"
)

func main(){

	utils.PrettyLog(consts.LOG_TYPE_INFO, "System starting")

	controllers.Start = time.Now()

	go func() {
		startMonitoring()
	}()

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

func startMonitoring(){
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Monitoring starting")
	for {

		resp, err := http.Get("http://192.168.220.128/data")
		if err != nil {
			utils.PrettyLog(consts.LOG_TYPE_ERROR, "Cannot contact device")
		}
		responseData, _ := ioutil.ReadAll(resp.Body)
		if len(responseData) < 1 {
			return
		}
		var m models.DeviceResponse
		err = json.Unmarshal(responseData, &m)
		if err != nil {
			utils.PrettyLog(consts.LOG_TYPE_ERROR, err.Error())
		}

		controllers.Values.Humidity = strconv.Itoa(m.Humid)
		controllers.Values.Temperature = strconv.Itoa(m.Temp)

		time.Sleep(time.Second * 2)
	}
}