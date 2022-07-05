from http.server import BaseHTTPRequestHandler, HTTPServer
from pickletools import long1
import xmlrpc.client
from time import time, sleep
import json
import copy
from datetime import datetime

# https://www.codexpedia.com/python/python-web-server-for-get-and-post-requests/
# https://stackoverflow.com/questions/36909047/do-post-not-working-on-http-server-with-python
# https://stackoverflow.com/questions/40557335/binary-to-string-text-in-python
serverPort = 8080


class MyServer(BaseHTTPRequestHandler):
    ## vars ##
    sensorValues = []
    rpcrtts = [] # save rpc call rtt here
    rpcserver = xmlrpc.client.ServerProxy('http://172.20.128.5:9000')
    rpcserver2 = xmlrpc.client.ServerProxy('http://172.20.128.42:9000')
    ## helper methods ##
    def getavgrpcrtt(self):
        
        duprtts = copy.deepcopy(self.rpcrtts) # get deep copy of array        
        total = 0
        if(len(duprtts) == 0):
            return 0, 0

        for t in duprtts:
            total += t
        return (total / len(duprtts)), total

    # Returns the current timestamp
    def getcurrtimestamp(self):
        return int(time() *1000000)

        

    # Generate a dict from timestamp and value
    def buildValueTimestampDict(self, _value:str, _timestamp:str):
        return dict(timestamp=_timestamp, value=_value)

    # Returns 2 strings: datetime formatted and messwert
    def getStringFromDataDict(self, data:dict):
        _dt = datetime.fromtimestamp(int(data['timestamp'])/1000000)# + timedelta(hours=6) # timezone offset
        _datestr = _dt.strftime("%H:%M:%S")
        return  _datestr, str(data['value'])

    def getRPCRTT(self, index : int):
        if(len(self.rpcrtts) > index):
            return self.rpcrtts[index]
        else:
            return -1

    def getTablestring(self):
        tableString = '<table id="valueTable"><tr><th>Time</th><th>Value</th><th>RPC rtt (usec)</th></tr>'
        # build html string
        i = 0 # accessor for rpcrtts
        for datadict in self.sensorValues:
            #Get the current dict values
            i += 1
            rpcrttstr = self.getRPCRTT(i)
            if(rpcrttstr == -1):
                rpcrttstr = "n/a"

            rowstring = "<tr>"
            clocktime, sensorval = self.getStringFromDataDict(datadict)
            rowstring += "<td>"+clocktime+"</td><td>"+sensorval+"</td><td>"+str(rpcrttstr)+"</td>"
            rowstring += "</tr>"
            # Add row to tablestr
            tableString += rowstring
        
        #Finally close the table
        tableString += "</table>"
        return tableString

    # Returns info as a jstring
    def getRPCTimesJSON(self):
        finalDict = dict(requestCooldown=0,name="RPC RTTs",sensornum=2, savedElements=len(self.rpcrtts), values=self.rpcrtts)
        return json.dumps(finalDict)

    def rpcOperationOK(self, resp):
        jresp = json.loads(resp)
        if(jresp['errno'] == 0):
            return True
        else:
            return False

    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    ## GET handler (Browser call) ##
    def do_GET(self):
        self._set_headers()
        
        if(self.path == "/sus"): # localhost:8080/sus
            jstring = self.getRPCTimesJSON()
            self.wfile.write(bytes(jstring,"utf-8"))
            return

        tableString = self.getTablestring()
        avgrpcrtt, totalrpcrtt = self.getavgrpcrtt() 
        #Write website
        self.wfile.write(bytes("<html><head><title>Amogus API</title><meta http-equiv='refresh' content='1'>", "utf-8"))
        # Styles
        self.wfile.write(bytes("<style>body{display:block;width:100%;}section{max-width:1280px;margin:0 auto;padding:0 2%;}p{display:block;font-weight:bold;word-break:break-all;font-style:italic;}", "utf-8"))
        self.wfile.write(bytes("table, th, td {border: 0px solid; width: 100%; text-align: center; padding-top: 4px; padding-bottom: 4px; font-family: monospace; font-size: 15px }", "utf-8"))
        self.wfile.write(bytes("#valueTable tr:nth-child(even){background-color: #6e6e6e; color: #ffffff}", "utf-8"))
        self.wfile.write(bytes("</style>", "utf-8"))
        #End styles
        self.wfile.write(bytes("</head>", "utf-8"))
        self.wfile.write(bytes("<body><section>", "utf-8"))
        #self.wfile.write(bytes("<h1>Request: %s</h1>" % self.path, "utf-8"))
        self.wfile.write(bytes("<h1>Stored data: {} key-value entries</h1>".format(len(self.sensorValues)), "utf-8"))
        self.wfile.write(bytes("<p>"+"RPC Average RTT: "+str(avgrpcrtt)+" usecs</p>", "utf-8"))
        self.wfile.write(bytes("<p>"+"RPC Total RTT: "+str(totalrpcrtt)+" usecs with "+str(len(self.rpcrtts))+"</p>", "utf-8"))
        self.wfile.write(bytes(tableString, "utf-8"))
        self.wfile.write(bytes("</section></body></html>", "utf-8"))

    ## POST handler (SocketSender call) ##
    def do_POST(self):
        content_length = int(self.headers['Content-Length']) # <--- Gets the size of data
        post_data = self.rfile.read(content_length) # <--- Gets the data itself
        
        # handle paramsgetcurrtimestamp
        recvString = post_data.decode('utf-8') # decode the byte-string with utf-8: b'89213892347929;123' => 89213892347929;123
        splitParams = recvString.split(';') # split the string with ';'
        passthroughTime = splitParams[0] # the timestamp
        # Output to POST
        print(recvString)
        print(splitParams)
        print("=====================================")
        # save to arrays
        # The array now contains dicts of (sensorval, timestamp) tuples
        self.sensorValues.append(self.buildValueTimestampDict(splitParams[1], splitParams[0]))
        
        # Send to RPC server
        # Check if both are online
        if(self.rpcOperationOK(self.rpcserver.check()) and self.rpcOperationOK(self.rpcserver2.check())):
            # Write to cache on rpcservers
            # Check response
            if(self.rpcOperationOK(self.rpcserver.write_to_cache(int(splitParams[1]), str(self.getcurrtimestamp()))) == True and True == self.rpcOperationOK(self.rpcserver.write_to_cache(int(splitParams[1]), str(self.getcurrtimestamp())))):
                __resp1 = json.loads(self.rpcserver.persist_cache(splitParams[0]))
                __resp2 = json.loads(self.rpcserver2.persist_cache(splitParams[0]))
                __longest = 0

                t1 = int(__resp1['value'])
                t2 = int(__resp2['value'])

                if(t1 > t2):
                    __longest = t1
                else:
                    __longest = t2

                dforthedelta = self.getcurrtimestamp() - int(__longest)
                self.rpcrtts.append(dforthedelta)

                print("Databases: Persisted!")
            else:
                # Rollback
                self.rpcserver.clear_cache()
                self.rpcserver2.clear_cache()  
   
                print("Database: Rollback!")
        else:
            pass
        # respond to get request
        self.wfile.write(passthroughTime.encode('utf-8'))

if __name__ == "__main__":
    # serve webserver on ''=>localhost through port 8080
    webServer = HTTPServer(('', serverPort), MyServer)
    print("Server started:{}".format(serverPort))

    try:
        webServer.serve_forever()
    except:
        print("Something went wrong!")
        pass
    webServer.server_close()
    print("Server stopped!")



    # nicht nur "text" empfangen sondern auch timestamps empfangen