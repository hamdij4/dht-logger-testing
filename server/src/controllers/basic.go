package controllers

import (
	"encoding/json"
	"fmt"
	"hamdij4/server/models"
	"net/http"
	"hamdij4/server/utils"
	"hamdij4/server/consts"
	"time"
)

var Values = new(models.DHTvalues)
var Start time.Time

// GET /online
func HandleOnlineCheck(w http.ResponseWriter, req *http.Request){
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Device online")
	fmt.Println(req.URL, req.Form)
	fmt.Fprintf(w, "Status - OK\n")
}
//POST /data?params
func HandleDataRX(w http.ResponseWriter, req *http.Request){
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Device RX")
	Values.Temperature = req.FormValue("temp")
	Values.Humidity = req.FormValue("humid")

	fmt.Fprintf(w, "Status - OK\n")
}
//GET /client/data
func HandleClientGET(w http.ResponseWriter, req *http.Request) {
	utils.PrettyLog(consts.LOG_TYPE_INFO, "Client TX")

	now := time.Now().Unix()
	Values.Uptime = now - Start.Unix()

	response, err  := json.Marshal(Values)
	if err != nil {
		utils.PrettyLog(consts.LOG_TYPE_WARN, "Could not marshal response for client")
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	w.WriteHeader(http.StatusOK)
	w.Write(response)
}

