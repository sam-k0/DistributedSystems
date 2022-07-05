from time import time
from datetime import datetime, timedelta

# Returns the current timestamp
def getcurrtimestamp():
    return int(time() * 1000)

# Generate a dict from timestamp and value
def buildValueTimestampDict(_value:str, _timestamp:str):
    return dict(timestamp=_timestamp, value=_value)

def getStringFromDataDict(data:dict):
    _str = "Time: "
    _dt = datetime.fromtimestamp(int(data['timestamp'])/1000)# + timedelta(hours=6) # timezone offset
    _datestr = _dt.strftime("%H:%M:%S")
    return _str +_datestr

# __main__
mydict = buildValueTimestampDict("69", str(getcurrtimestamp()))
print(getStringFromDataDict(mydict))
