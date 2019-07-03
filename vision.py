

import vrep
import numpy as np
import matplotlib.pyplot as mlp
import time
print('Program started')
vrep.simxFinish(-1) # just in case, close all opened connections
clientID=vrep.simxStart('127.0.0.1',19999,True,True,5000,5)
if clientID!=-1:
    
    print('Connected to remote API server')

    # err_code,l_motor_handle = vrep.simxGetObjectHandle(clientID,"bubbleRob_leftMotor", vrep.simx_opmode_blocking)
    # err_code,r_motor_handle = vrep.simxGetObjectHandle(clientID,"bubbleRob_rightMotor", vrep.simx_opmode_blocking)

    # err_code = vrep.simxSetJointTargetVelocity(clientID,l_motor_handle,1.0,vrep.simx_opmode_streaming)
    # err_code = vrep.simxSetJointTargetVelocity(clientID,r_motor_handle,1.0,vrep.simx_opmode_streaming)

    # i = 0

    # err_code,camera = vrep.simxGetObjectHandle(clientID,"Vision_sensor",vrep.simx_opmode_blocking)
    # print(err_code)
    # err_code,resolution,image = vrep.simxGetVisionSensorImage(clientID,camera,0,vrep.simx_opmode_streaming)
    # print(err_code)

    # while (vrep.simxGetConnectionId(clientID)!=-1):
    #     err_code,resolution,image = vrep.simxGetVisionSensorImage(clientID,camera,0,vrep.simx_opmode_buffer)
    #     if (i < 10):
    #         print(image)
    #         i+=1
    #     else:
    #         break

    err_code,l_motor_handle = vrep.simxGetObjectHandle(clientID,"ePuck_leftJoint", vrep.simx_opmode_blocking)
    err_code,r_motor_handle = vrep.simxGetObjectHandle(clientID,"ePuck_rightJoint", vrep.simx_opmode_blocking)

    err_code = vrep.simxSetJointTargetVelocity(clientID,l_motor_handle,-1.0,vrep.simx_opmode_streaming)
    err_code = vrep.simxSetJointTargetVelocity(clientID,r_motor_handle,-1.0,vrep.simx_opmode_streaming)

    i = 0

    err_code,camera = vrep.simxGetObjectHandle(clientID,"ePuck_camera",vrep.simx_opmode_blocking)
    print(err_code)
    err_code,resolution,image = vrep.simxGetVisionSensorImage(clientID,camera,0,vrep.simx_opmode_streaming)
    print(err_code)

    # while (vrep.simxGetConnectionId(clientID)!=-1):
    #     err_code,resolution,image = vrep.simxGetVisionSensorImage(clientID,camera,0,vrep.simx_opmode_buffer)
    #     if (i < 10):
    #         print(image)
    #         i+=1
    #     else:
    #         break

    


else:
    print('Failed connecting to remote API server')
print('Program ended')
