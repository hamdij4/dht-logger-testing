package models

type DHTvalues struct {
	Temperature string
	Humidity string
	Uptime int64
}

type DeviceResponse struct {
	Humid int `json: 'humid''`
	Temp int `json: 'temp''`
}