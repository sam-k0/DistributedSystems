import xmlrpc.client
s = xmlrpc.client.ServerProxy('http://localhost:9000')

print(s.insert(1234567,"0934590768"))
print(s.read(7))

print(s.insert(939393, "0010101"))
print(s.read(0))
print(s.delete(0))

# Print list of available methods
print(s.system.listMethods())