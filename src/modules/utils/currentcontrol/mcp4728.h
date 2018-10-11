#ifndef MCP4728_H
#define MCP4728_H

#include "libs/Kernel.h"
#include "I2C.h" // mbed.h lib
#include "libs/utils.h"
#include "DigipotBase.h"
#include <string>
#include <math.h>

class MCP4728 : public DigipotBase {
    public:
        MCP4728(){
            // I2C com
            this->i2c = new mbed::I2C(p9, p10);
            this->i2c->frequency(2000);
            for (int i = 0; i < 8; i++)
                currents[i] = 0.0;
        }

        ~MCP4728(){
            delete this->i2c;
        }

        void set_current( int channel, float current )
        {
            current = min( max( current, 0.0f ), this->max_current );	// Brilliant way to check if current > maximum
            currents[channel] = current;	// Set channel current
	    char addr = 0xC0;
            if(channel > 3) {addr = 0xC2;}				// I2C MCP4728 address if it's bought as Addr 000.
            // Address for channels MCP4728
            char addresses[8] = { 0x46, 0x44, 0x42, 0x40, 0x46, 0x44, 0x42, 0x40 };	// Channels A-H, UDAC bit updates channels
	    this->i2c_send( addr, addresses[channel], this->current_to_dac(current) );
        }

        float get_current(int channel)
        {
            return currents[channel];
        }

    private:

        void i2c_send( char i2c_addr, char dac_addr, int current_setting ){            
	    this->i2c->start();
            this->i2c->write(i2c_addr);					// Point to DAC
            this->i2c->write(dac_addr);					// Point to specific DAC, internal Ref, blah
            this->i2c->write(((current_setting>>8)&0x8F)|0x80);	// Set first nibble (msb of current)
	    this->i2c->write(current_setting);			// LSB to currentsetting
            this->i2c->stop();
        }

		int current_to_dac(double current){
			return (int)(ceil(double(current*this->factor)));		// Factor is 800(A4988/A4982).. 1000(DRV8825), set in config.
		}
		
        mbed::I2C* i2c;
        float currents[8];
};


#endif
