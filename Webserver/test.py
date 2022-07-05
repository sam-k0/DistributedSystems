import datetime
import time

currdateusec = datetime.datetime.now().microsecond
millisecs = int(time.time() * 1000000)
combined = (millisecs*1000) + (currdateusec/1000)
print(currdateusec)
print(millisecs)
print(int(time.time() * 1000))
print(int(combined))