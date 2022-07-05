
import matplotlib.pyplot as plt
import json
import os

# load json contents to file
filenames = ["sensortimes.json","rpcrtts.json", "httptimes.json"]
xlabelname = "Package"
ylabelname = "RTTs in USEC"


def plotFile(fname: str):
    # File handling
    jsonfilepath = os.getcwd() + os.sep + "Plotting" + os.sep +"reports"+os.sep+ fname
    print("Searching file in "+jsonfilepath)
    # Open and read
    file = open(jsonfilepath, "r")
    datastr = file.read()
    file.close()
    # Serialize JSON
    jsondict = json.loads(datastr)
    # Read dict contents
    numClients = jsondict['sensornum']
    numEntries = jsondict['savedElements']
    readValues = jsondict['values']
    plotname = jsondict['name']
    requestCooldown = jsondict['requestCooldown']

    plotname += ": number of clients: {}, request cooldown: {} usec".format(numClients, requestCooldown)

    if(numEntries != len(readValues)):
        print("The element number differs! {}/{}".format(numEntries, len(readValues)))
        exit(-99)
    # avg time
    avgrtt = 0
    for i in readValues:
        avgrtt += i
    avgrtt = avgrtt / len(readValues)

    # plotting begin
    xvalues = []
    # fill with package placeholders
    for i in range(numEntries):
        #print(i)
        xvalues.append(i)

    plt.plot(xvalues, readValues)
    plt.xlabel(xlabelname)
    plt.ylabel(ylabelname)
    plt.title(plotname + " Avg RTT: " + str(avgrtt) + " usec.")
    plt.show()

for cfname in filenames:
    
    print("Plotting "+os.getcwd() + os.sep + "Plotting" + os.sep +"reports"+os.sep+ cfname)
    try:
        plotFile(cfname)
    except:
        print("File not found / Couldnt be opened")

