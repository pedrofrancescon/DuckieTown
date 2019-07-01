// Copyright 2006-2016 Coppelia Robotics GmbH. All rights reserved.
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
//
// -------------------------------------------------------------------
// THIS FILE IS DISTRIBUTED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
// WARRANTY. THE USER WILL USE IT AT HIS/HER OWN RISK. THE ORIGINAL
// AUTHORS AND COPPELIA ROBOTICS GMBH WILL NOT BE LIABLE FOR DATA LOSS,
// DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF LOSS WHILE USING OR
// MISUSING THIS SOFTWARE.
//
// You are free to use/modify/distribute this file for whatever purpose!
// -------------------------------------------------------------------
//
// This file was automatically created for V-REP release V3.3.0 on February 19th 2016

// Make sure to have the server side running in V-REP!
// Start the server from a child script with following command:
// simExtRemoteApiStart(portNumber) -- starts a remote API server service on the specified port

#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <time.h>
#include "flie.h"

extern "C" {
    #include "extApi.h"
}

//#include "BMP.h"

/* BEGIN BMP.H */

#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <stdint.h>

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t file_type;               // File type always BM which is 0x4D42 (stored as hex uint16_t in little endian)
    uint32_t file_size;               // Size of the file (in bytes)
    uint16_t reserved1;               // Reserved, always 0
    uint16_t reserved2;               // Reserved, always 0
    uint32_t offset_data;             // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader {
    uint32_t size;                      // Size of this header (in bytes)
    int32_t width;                      // width of bitmap in pixels
    int32_t height;                     // width of bitmap in pixels
                                        //       (if positive, bottom-up, with origin in lower left corner)
                                        //       (if negative, top-down, with origin in upper left corner)
    uint16_t planes;                    // No. of planes for the target device, this is always 1
    uint16_t bit_count;                 // No. of bits per pixel
    uint32_t compression;               // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
    uint32_t size_image;                // 0 - for uncompressed images
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;               // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t colors_important;          // No. of colors used for displaying the bitmap. If 0 all colors are required
};

struct BMPColorHeader {
    uint32_t red_mask;         // Bit mask for the red channel
    uint32_t green_mask;       // Bit mask for the green channel
    uint32_t blue_mask;        // Bit mask for the blue channel
    uint32_t alpha_mask;       // Bit mask for the alpha channel
    uint32_t color_space_type; // Default "sRGB" (0x73524742)
    uint32_t unused[16];       // Unused data for sRGB color space
};
#pragma pack(pop)

struct BMP {
    BMPFileHeader file_header;
    BMPInfoHeader bmp_info_header;
    BMPColorHeader bmp_color_header;
    std::vector<uint8_t> data;

    void startValues(){

        //file_header
        file_header.file_type = 0x4D42;
        file_header.file_size = 0;
        file_header.reserved1 = 0;
        file_header.reserved2 = 0;
        file_header.offset_data = 0;

        //bmp_info_header
        bmp_info_header.size = 0;
        bmp_info_header.width = 0;
        bmp_info_header.height = 0;

        bmp_info_header.planes = 1;
        bmp_info_header.bit_count = 0;
        bmp_info_header.compression = 0;
        bmp_info_header.size_image = 0;
        bmp_info_header.x_pixels_per_meter = 0;
        bmp_info_header.y_pixels_per_meter = 0;
        bmp_info_header.colors_used = 0;
        bmp_info_header.colors_important = 0;

        //bmp_color_header
        bmp_color_header.red_mask = 0x00ff0000;
        bmp_color_header.green_mask = 0x0000ff00;
        bmp_color_header.blue_mask = 0x000000ff;
        bmp_color_header.alpha_mask = 0xff000000;
        bmp_color_header.color_space_type = 0x73524742;
        for (int i=0; i < 16; i++) {
            bmp_color_header.unused[i] = 0;
        }

        //private members
        row_stride = 0;
    }

    BMP(const char *fname) {
        startValues();
        read(fname);
    }

    void read(const char *fname) {
        std::ifstream inp (fname, std::ios_base::binary);
        if (inp) {
            inp.read((char*)&file_header, sizeof(file_header));
            if(file_header.file_type != 0x4D42) {
                throw std::runtime_error("Error! Unrecognized file format.");
            }
            inp.read((char*)&bmp_info_header, sizeof(bmp_info_header));

            // The BMPColorHeader is used only for transparent images
            if(bmp_info_header.bit_count == 32) {
                // Check if the file has bit mask color information
                if(bmp_info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
                    inp.read((char*)&bmp_color_header, sizeof(bmp_color_header));
                    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                    check_color_header(bmp_color_header);
                } else {
                    std::cerr << "Error! The file \"" << fname << "\" does not seem to contain bit mask information\n";
                    throw std::runtime_error("Error! Unrecognized file format.");
                }
            }

            // Jump to the pixel data location
            inp.seekg(file_header.offset_data, inp.beg);

            // Adjust the header fields for output.
            // Some editors will put extra info in the image file, we only save the headers and the data.
            if(bmp_info_header.bit_count == 32) {
                bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            } else {
                bmp_info_header.size = sizeof(BMPInfoHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
            }
            file_header.file_size = file_header.offset_data;

            if (bmp_info_header.height < 0) {
                throw std::runtime_error("The program can treat only BMP images with the origin in the bottom left corner!");
            }

            data.resize(bmp_info_header.width * bmp_info_header.height * bmp_info_header.bit_count / 8);

            // Here we check if we need to take into account row padding
            if (bmp_info_header.width % 4 == 0) {
                inp.read((char*)data.data(), data.size());
                file_header.file_size += data.size();
            }
            else {
                row_stride = bmp_info_header.width * bmp_info_header.bit_count / 8;
                uint32_t new_stride = make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - row_stride);

                for (int y = 0; y < bmp_info_header.height; ++y) {
                    inp.read((char*)(data.data() + row_stride * y), row_stride);
                    inp.read((char*)padding_row.data(), padding_row.size());
                }
                file_header.file_size += data.size() + bmp_info_header.height * padding_row.size();
            }
        }
        else {
            throw std::runtime_error("Unable to open the input image file.");
        }
    }

    BMP(int32_t width, int32_t height, bool has_alpha = true) {
        startValues();
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("The image width and height must be positive numbers.");
        }

        bmp_info_header.width = width;
        bmp_info_header.height = height;
        if (has_alpha) {
            bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);

            bmp_info_header.bit_count = 32;
            bmp_info_header.compression = 3;
            row_stride = width * 4;
            data.resize(row_stride * height);
            file_header.file_size = file_header.offset_data + data.size();
        }
        else {
            bmp_info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmp_info_header.bit_count = 24;
            bmp_info_header.compression = 0;
            row_stride = width * 3;
            data.resize(row_stride * height);

            uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = file_header.offset_data + data.size() + bmp_info_header.height * (new_stride - row_stride);
        }
    }

    void write(const char *fname) {
        std::ofstream of (fname, std::ios_base::binary);
        if (of) {
            if (bmp_info_header.bit_count == 32) {
                write_headers_and_data(of);
            }
            else if (bmp_info_header.bit_count == 24) {
                if (bmp_info_header.width % 4 == 0) {
                    write_headers_and_data(of);
                }
                else {
                    uint32_t new_stride = make_stride_aligned(4);
                    std::vector<uint8_t> padding_row(new_stride - row_stride);

                    write_headers(of);

                    for (int y = 0; y < bmp_info_header.height; ++y) {
                        of.write((const char*)(data.data() + row_stride * y), row_stride);
                        of.write((const char*)padding_row.data(), padding_row.size());
                    }
                }
            }
            else {
                throw std::runtime_error("The program can treat only 24 or 32 bits per pixel BMP files");
            }
        }
        else {
            throw std::runtime_error("Unable to open the output image file.");
        }
    }

    void fill_region(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        if (x0 + w > (uint32_t)bmp_info_header.width || y0 + h > (uint32_t)bmp_info_header.height) {
            throw std::runtime_error("The region does not fit in the image!");
        }

        uint32_t channels = bmp_info_header.bit_count / 8;
        for (uint32_t y = y0; y < y0 + h; ++y) {
            for (uint32_t x = x0; x < x0 + w; ++x) {
                data[channels * (y * bmp_info_header.width + x) + 0] = B;
                data[channels * (y * bmp_info_header.width + x) + 1] = G;
                data[channels * (y * bmp_info_header.width + x) + 2] = R;
                if (channels == 4) {
                    data[channels * (y * bmp_info_header.width + x) + 3] = A;
                }
            }
        }
    }

    void fill_pixel(uint32_t x, uint32_t y, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        if (x > (uint32_t)bmp_info_header.width || y > (uint32_t)bmp_info_header.height) {
            throw std::runtime_error("The region does not fit in the image!");
        }

        uint32_t channels = bmp_info_header.bit_count / 8;
        data[channels * (y * bmp_info_header.width + x) + 0] = B;
        data[channels * (y * bmp_info_header.width + x) + 1] = G;
        data[channels * (y * bmp_info_header.width + x) + 2] = R;
        if (channels == 4) {
            data[channels * (y * bmp_info_header.width + x) + 3] = A;
        }
    }

    void draw_rectangle(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h,
                        uint8_t B, uint8_t G, uint8_t R, uint8_t A, uint8_t line_w) {
        if (x0 + w > (uint32_t)bmp_info_header.width || y0 + h > (uint32_t)bmp_info_header.height) {
            throw std::runtime_error("The rectangle does not fit in the image!");
        }

        fill_region(x0, y0, w, line_w, B, G, R, A);                                             // top line
        fill_region(x0, (y0 + h - line_w), w, line_w, B, G, R, A);                              // bottom line
        fill_region((x0 + w - line_w), (y0 + line_w), line_w, (h - (2 * line_w)), B, G, R, A);  // right line
        fill_region(x0, (y0 + line_w), line_w, (h - (2 * line_w)), B, G, R, A);                 // left line
    }

private:
    uint32_t row_stride;

    void write_headers(std::ofstream &of) {
        of.write((const char*)&file_header, sizeof(file_header));
        of.write((const char*)&bmp_info_header, sizeof(bmp_info_header));
        if(bmp_info_header.bit_count == 32) {
            of.write((const char*)&bmp_color_header, sizeof(bmp_color_header));
        }
    }

    void write_headers_and_data(std::ofstream &of) {
        write_headers(of);
        of.write((const char*)data.data(), data.size());
    }

    // Add 1 to the row_stride until it is divisible with align_stride
    uint32_t make_stride_aligned(uint32_t align_stride) {
        uint32_t new_stride = row_stride;
        while (new_stride % align_stride != 0) {
            new_stride++;
        }
        return new_stride;
    }

    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
    void check_color_header(BMPColorHeader &bmp_color_header) {
        BMPColorHeader expected_color_header;
        expected_color_header.red_mask = 0x00ff0000;
        expected_color_header.green_mask = 0x0000ff00;
        expected_color_header.blue_mask = 0x000000ff;
        expected_color_header.alpha_mask = 0xff000000;
        expected_color_header.color_space_type = 0x73524742;
        for (int i=0; i < 16; i++) {
            expected_color_header.unused[i] = 0;
        }


        if(expected_color_header.red_mask != bmp_color_header.red_mask ||
            expected_color_header.blue_mask != bmp_color_header.blue_mask ||
            expected_color_header.green_mask != bmp_color_header.green_mask ||
            expected_color_header.alpha_mask != bmp_color_header.alpha_mask) {
            throw std::runtime_error("Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
        }
        if(expected_color_header.color_space_type != bmp_color_header.color_space_type) {
            throw std::runtime_error("Unexpected color space type! The program expects sRGB values");
        }
    }
};

/* END BMP.H */

//calcular a posicao no vetor de imagem, para um pixel em "linha,coluna". Inicio embaixo a esquerda, uma linha por vez, até o topo a direita. Canal 0=R,1=G,2=B
int getposition(int linha, int coluna, int canal, int resolucaoX, int resolucaoY)
{
 if((linha<(resolucaoX))&&(coluna<(resolucaoY)))
 return(linha*((resolucaoX-1)*3)+coluna*3+canal);
 else
 return(0);
}

int getpos(int lin, int col, int canal)
{
 return(getposition(lin, col, canal, 128, 128));
}

// Variables for FLIE framework
fuzzy_control fc;
fuzzy_set fs_near,fs_far, fs_soft, fs_hard;
linguisticvariable Distance, BreakControl;
rule infrule[2];


int main(int argc,char* argv[])
{
	int portNb=0;
	int leftMotorHandle;
	int rightMotorHandle;
	int sensorHandle[6];
	float distanceSensor[6];
        int count=0;

        //variáveis relativas ao controle de tempo para salvar os arquivos
        double time_counter = 0;
        double wait_seconds = 1;
        double clocks_per_sec = 20000;

        clock_t this_time = clock();
        clock_t last_time = this_time;
        //variáveis relativas ao controle de tempo para salvar os arquivos

	if (argc>=5)
	{
		portNb=atoi(argv[1]);
		leftMotorHandle=atoi(argv[2]);
		rightMotorHandle=atoi(argv[3]);
		sensorHandle[0]=atoi(argv[4]);
		sensorHandle[1]=atoi(argv[5]);
		sensorHandle[2]=atoi(argv[6]);
		sensorHandle[3]=atoi(argv[7]);
		sensorHandle[4]=atoi(argv[8]);
		sensorHandle[5]=atoi(argv[9]);
	}
	else
	{
		printf("Indicate following arguments: 'portNumber leftMotorHandle rightMotorHandle sensorHandle[1..6]'!\n");
		extApi_sleepMs(5000);
		return 0;
	}
	printf("Handles: portNumber=%d leftMotorHandle=%d rightMotorHandle=%d sensorHandle[1]=%d sensorHandle[2]=%d sensorHandle[3]=%d sensorHandle[4]=%d sensorHandle[5]=%d sensorHandle[6]=%d\n", portNb, leftMotorHandle, rightMotorHandle, sensorHandle[0],sensorHandle[1],sensorHandle[2],sensorHandle[3], sensorHandle[4],sensorHandle[5]);

	int clientID=simxStart((simxChar*)"127.0.0.1",portNb,true,true,2000,5);
	if (clientID!=-1)
	{

		for(int i = 0; i < 6; i++)
			simxReadProximitySensor(clientID,sensorHandle[i],NULL,NULL,NULL,NULL,simx_opmode_streaming);
		int driveBackStartTime=-99000;
		float motorSpeeds[2];

		while (simxGetConnectionId(clientID)!=-1)
		{
			/*simxUChar sensorTrigger=0;
			if (simxReadProximitySensor(clientID,sensorHandle[4],&sensorTrigger,NULL,NULL,NULL,simx_opmode_streaming)==simx_return_ok)
			{ // We succeeded at reading the proximity sensor
				int simulationTime=simxGetLastCmdTime(clientID);
				if (simulationTime-driveBackStartTime<3000)
				{ // driving backwards while slightly turning:
					motorSpeeds[0]=-3.1415f*0.5f;
					motorSpeeds[1]=-3.1415f*0.25f;
				}
				else
				{ // going forward:
					motorSpeeds[0]=3.1415f;
					motorSpeeds[1]=3.1415f;
					if (sensorTrigger)
						driveBackStartTime=simulationTime; // We detected something, and start the backward mode
				}

			}*/
		    int passive;

		    simxGetObjectHandle(clientID,"ePuck_camera",&passive,simx_opmode_blocking);

                    int resolution[2];
		    simxUChar* image;
		    simxGetVisionSensorImage(clientID,passive,resolution,&image,0,simx_opmode_streaming);

	            if (simxGetVisionSensorImage(clientID,passive,resolution,&image,0,simx_opmode_buffer)==0)
	            {
//                      printf("Resolution: %d x %d", resolution[0], resolution[1]);
//                      printf("Data:image[1][1](RGB) = %d,%d,%d\n", image[0],image[1],image[2]);
//                      printf("Data:(canto inferior direito) image[][](RGB) -> image[%d]=%d,image[%d]=%d,image[%d]=%d\n", 127*3+0, image[127*3+0],127*3+1, image[127*3+1],127*3+2, image[127*3+2]);
//	                printf("Data:(canto inferior direito) image[][](RGB) -> %d,%d,%d\n", image[getpos(0,127,0)], image[getpos(0,127, 1)], image[getpos(0,127,2)]);
//	                printf("Data:(canto superior esquerdo) image[][](RGB)-> image[%d]=%d,image[%d]=%d,image[%d]=%d\n",resolution[0]*3*(resolution[1]-1)+0, image[resolution[0]*3*(resolution[1]-1)+0],resolution[0]*3*(resolution[1]-1)+1,image[resolution[0]*3*(resolution[1]-1)+1],resolution[0]*3*(resolution[1]-1)+2, image[resolution[0]*3*(resolution[1]-1)+2]);
//	                printf("Data:(canto superior direito) image[][](RGB) -> image[%d]=%d,image[%d]=%d,image[%d]=%d\n", 128*3*128-3, image[128*3*128-3], 128*3*128-2, image[128*3*128-2], 128*3*128-1, image[128*3*128-1]);
//	                printf("Data:(canto superior direito) image[][](RGB) -> %d,%d,%d\n", image[getpos(127,127,0)], image[getpos(127,127, 1)], image[getpos(127,127,2)]);

                        this_time = clock();
                        time_counter += (double)(this_time - last_time);
                        last_time = this_time;

                        if(time_counter > (double)(wait_seconds * clocks_per_sec))
                        {
                            time_counter -= (double)(wait_seconds * clocks_per_sec);

                            //BMP imagem da câmera
                            int lar = 128;
                            int alt = 64;
                            BMP bmp_camera(lar, alt);

                            int image_pos = 0;
                            for (int y = 0; y < alt; y++) {
                                for (int x = 0; x < lar; x++) {
                                    bmp_camera.fill_pixel(x, y, image[image_pos+2] ,image[image_pos+1] ,image[image_pos], 255);
                                    image_pos += 3;
                                }
                            }

                            //construção da string do nome do arquivo:
                            //pasta imagens (no meu computador, linux, criei esta pasta dentro da pasta do VREP (Ex: ~/vrep/imagens/)
                            char str[40] = "imagens/";

                            //gera string datetime
                            std::time_t t = std::time(0);
                            std::string tempo = std::asctime(localtime(&t));
                            //workaround para remover \n do final da string
                            tempo = tempo.substr(0, tempo.size()-1);

                            strcat(str, tempo.c_str());
                            strcat(str, ".bmp");
                            bmp_camera.write(str);
                            std::cout << "Arquivo salvo: " << str << std::endl;
                        }
                    }
		float distance;
	        float distanceAxis[4];
		unsigned char is_there_obstacle[6];

		for(int i = 0; i < 6; i++){
			distanceAxis[0]=0.0;
			distanceAxis[1]=0.0;
			distanceAxis[2]=0.0;
			distance=0.0;
			distanceSensor[i]=10.0;

       			simxReadProximitySensor(clientID,sensorHandle[i],&is_there_obstacle[i],distanceAxis,NULL,NULL,simx_opmode_buffer);

			if(is_there_obstacle[i]==1){
				distance = pow(distanceAxis[0],2) + pow(distanceAxis[1],2) + pow(distanceAxis[2],2);
	       			distance = sqrt(distance);
				distanceSensor[i]=distance;
			}
			if((distanceSensor[i]<0.001)||(distanceSensor[i]>=10.0)) distanceSensor[i]=1.0;
			if(distanceSensor[i]!=1.0) {count++;printf("%d = %f \t", i, distanceSensor[i]);}
			}
		if(count>0){count=0;printf("\n");}
		
        
        if(distanceSensor[4]<0.032)
		{
			motorSpeeds[0]=-3.1415f;
			motorSpeeds[1]=3.1415f;
		}
		else
		{		
			motorSpeeds[0]=3.1415f;
			motorSpeeds[1]=3.1415f;
		}


		simxSetJointTargetVelocity(clientID,leftMotorHandle,motorSpeeds[0],simx_opmode_oneshot);			
		simxSetJointTargetVelocity(clientID,rightMotorHandle,motorSpeeds[1],simx_opmode_oneshot);			
		
		extApi_sleepMs(50);
	}
	simxFinish(clientID);
 	}
	return(0);
}

