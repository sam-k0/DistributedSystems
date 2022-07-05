from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.server import SimpleXMLRPCRequestHandler
import json
import socket
# Database array
DB_Array = []
Cache_Array = [] 
port = 9000

print("Starting XMLRPC server")

# Get default dict
def build_response(responseString: str, errno: int, value: str):
    return {"response": responseString, "errno": errno, "value": value}

def resp2str(amogus):
    return json.dumps(amogus)

class RequestHandler(SimpleXMLRPCRequestHandler):
   rpc_paths = ('/RPC2',)
#


with SimpleXMLRPCServer(('172.20.128.5', port),requestHandler=RequestHandler) as server:
    server.register_introspection_functions() # Essential default functions

    ## INSERT Returns default dict : Error codes begin with 1
    @server.register_function(name='insert')
    def db_insert(_value: int, _timestamp: str):
        # Append to dict
        DB_Array.append(dict(value=_value, timestamp=_timestamp))

        return resp2str(build_response("InsertSuccess", 0, str(_timestamp)))

    ## READ Returns: Stringified dict : Err codes begin with 2
    @server.register_function(name='read')
    def db_read(_index: int):
        
        if _index == -1: # Return all values
            return resp2str(build_response("ReturnAllValues", 0, str(DB_Array)))
        elif _index >= 0:# Return specific
            if len(DB_Array) >= _index: # in bounds
                return resp2str(build_response("ReturnSingleValue", 0, str(DB_Array[_index])))
            else:
                return resp2str(build_response("ReadErrorOOB", 21, ""))

    ## DELETE Returns: Stringified dictionary : error codes begin with 3
    @server.register_function(name='delete')
    def db_delete(_index: int):
        if _index == -1:
            DB_Array.clear() # Clear array
            return resp2str(build_response("ClearAllSuccess", 0, ""))
        elif _index >= 0:
            if len(DB_Array) >= _index:
                _deletedDict = DB_Array.pop(_index) # pop from pos                                
                return resp2str(build_response("DeleteSingleValue", 0, str(_deletedDict)))
            else:
                return resp2str(build_response("DeleteSingleOOB", 31, ""))

    # Write to cache
    @server.register_function
    def write_to_cache(_value: int, _timestamp: str):
        Cache_Array.append(dict(value=_value, timestamp=_timestamp))
        return resp2str(build_response("AddCacheSuccess", 0, str(_timestamp)))

    # Persist cache
    @server.register_function
    def persist_cache(timestamp: str):
        for element in Cache_Array:
            DB_Array.append(element)
        Cache_Array.clear()
        return resp2str(build_response("PersistCacheSuccess", 0, timestamp))

    # Clear cache
    @server.register_function
    def clear_cache():
        Cache_Array.clear()
        return resp2str(build_response("ClearCacheSuccess", 0, ""))

    # Still alive?
    @server.register_function(name='check')
    def vibecheck():
        return resp2str(build_response("PersistCacheSuccess", 0, "0"))
    
    # Connection testing
    @server.register_function
    def timetest(x):
        return x

    # Never stop running
    server.serve_forever()