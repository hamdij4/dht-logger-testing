package utils

import (
	"fmt"
	"time"
)

func PrettyLog(logType string, logData... string){
	fmt.Println("[" , time.Now().Format(time.RFC1123) , "]" , " [" , logType , "] " , logData)
}