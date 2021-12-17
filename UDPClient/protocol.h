/*
 * protocol.h
 *
 *  Created on: 16 dic 2021
 *      Author: Windows 10
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

 #define DEFAULT_PORT 27015
    #define DEFAULT_SERVER "localhost"
    #define BUFFER_SIZE 512
    #define MAX_ARGC 2 // max arguments
    #define DEFAULT_BEHAVIOUR 0

    typedef struct {
        union {
            int integer;
            float real;
        };
        char type;
    } result;

    typedef struct {
        char symbol;
        int first;
        int second;
    } operation;


    #define TYPE_INT 0x0
    #define TYPE_FLOAT 0x1

    char allowed_operations[] = "+-/x=";


    result add(int a, int b);
    result sub(int a, int b);
    result mult(int a, int b);
    result division(int a, int b);


#endif /* PROTOCOL_H_ */
