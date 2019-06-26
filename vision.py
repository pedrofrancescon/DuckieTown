print 'Program started'
vrep.simxFinish(-1) # just in case, close all opened connections
clientID=vrep.simxStart('127.0.0.1',19999,True,True,5000,5)
if clientID!=-1:
    
    print 'Connected to remote API server'
    res,v0=vrep.simxGetObjectHandle(clientID,'NAO_vision1',vrep.simx_opmode_oneshot_wait)
    
    res,resolution,image=vrep.simxGetVisionSensorImage(clientID,v0,0,vrep.simx_opmode_streaming)
    
    while (vrep.simxGetConnectionId(clientID)!=-1):
        res,resolution,image=vrep.simxGetVisionSensorImage(clientID,v0,0,vrep.simx_opmode_buffer)

else:
    print 'Failed connecting to remote API server'
print 'Program ended'
