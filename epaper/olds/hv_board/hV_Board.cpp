#include <hv_board/hV_Board.h>
#include <tyme/tyme.h>
#include <hv_board/hV_List_Constants.h>


namespace EPAPER{




hV_Board_t::hV_Board_t(uint32_t eScreen_EPD, const pins_t b_board)
    :       Gpio_t          ( enable )
        ,   b_pin           { b_board }
        ,   b_delayCS       { 1 }
        ,   spi_ptr         { std::make_unique<SPI::Spi_t>() }
        
    {	
        //b_begin( b_pin ,  eScreen_EPD,   1);
        //COG_SmallCJ_initial();
        //COG_SmallCJ_powerOff();

        COG_SmallCJ_initial(); // Initialise
        COG_SmallCJ_sendImageData(); // Send image data
        COG_SmallCJ_update(); // Update        
        COG_SmallCJ_powerOff(); // Power 
        TYME::delay_ms(500);
        //u_bufferDepth = v_screenColourBits; // 2 colours
        //u_bufferSizeV = v_screenSizeV; // vertical = wide size
        //u_bufferSizeH = v_screenSizeH / 8; // horizontal = small size 112 / 8, 1 bit per pixel
        b_resume();
        COG_SmallCJ_update();
        TYME::delay_ms(1000);
        b_resume();
    }

    void hV_Board_t::b_resume(){
        // Target FSM_ON
        // Source FSM_SLEEP -> FSM_SLEEP
        //        FSM_OFF   -> FSM_SLEEP
    
        if ((b_fsmPowerScreen & FSM_GPIO_MASK) != FSM_GPIO_MASK)
        {
        
            // Configure GPIOs
            pinMode(b_pin.panelBusy, INPUT);
    
            pinMode(b_pin.panelDC, OUTPUT);
            digitalWrite(b_pin.panelDC, HIGH);
    
            pinMode(b_pin.panelReset, OUTPUT);
            digitalWrite(b_pin.panelReset, HIGH);
    
            pinMode(b_pin.panelCS, OUTPUT);
            digitalWrite(b_pin.panelCS, HIGH); // CS# = 1
    
    
    
            // External SPI memory
            if (b_pin.flashCS != NOT_CONNECTED) // generic
            {
                pinMode         ( b_pin.flashCS, OUTPUT);
                digitalWrite    ( b_pin.flashCS, HIGH);
            }
    
    
            b_fsmPowerScreen |= FSM_GPIO_MASK;
        }
    }




    void hV_Board_t::b_sendCommand8(uint8_t command){
        digitalWrite(b_pin.panelDC, LOW);
        digitalWrite(b_pin.panelCS, LOW);
        hV_HAL_SPI_transfer(command);
        digitalWrite(b_pin.panelCS, HIGH);
    }


    void hV_Board_t::b_sendCommandData8(uint8_t command, uint8_t data)
    {
        digitalWrite( b_pin.panelDC , LOW); // LOW = command
        digitalWrite( b_pin.panelCS , LOW);

        hV_HAL_SPI_transfer( command );

        digitalWrite(b_pin.panelDC, HIGH); // HIGH = data
        hV_HAL_SPI_transfer(data);

        digitalWrite(b_pin.panelCS, HIGH);
    }




    uint8_t hV_Board_t::hV_HAL_SPI_transfer( uint8_t command){
        return spi_ptr->Transfer1bytes(command);
    }


    void hV_Board_t::COG_SmallCJ_initial()
    {
        // Application note § 3. Set environment temperature and PSR

        // Soft reset
        // Work settings
        b_reset(5, 5, 10, 5, 5); 

        b_sendCommandData8(0x00, 0x0e); // Soft-reset
        TYME::delay_ms(5);

        // Temperature
        b_sendCommandData8(0xe5, u_temperature); // Input Temperature 0°C = 0x00, 22°C = 0x16, 25°C = 0x19
        b_sendCommandData8(0xe0, 0x02); // Active Temperature
        b_sendIndexData(0x00, COG_data, 2); // PSR
    }




    void hV_Board_t::COG_SmallCJ_powerOff(){
        // Application note § 5. Turn-off DC/DC

        // DC-DC off
        b_sendCommand8(0x02); // Turn off DC/DC
        b_waitBusy();
    }



    void hV_Board_t::b_waitBusy(bool state){
    while (digitalRead(b_pin.panelBusy) != state)
        {
            TYME::delay_ms(32); // non-blocking
        }
    }
    
    
    void hV_Board_t::b_reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5){
    	// note: group delays into one array
    	TYME::delay_ms(ms1);    /// 5 msec 
        digitalWrite(b_pin.panelReset, HIGH); // RES# = 1
        TYME::delay_ms(ms2);    /// 5 msec 
        digitalWrite(b_pin.panelReset, LOW);
        TYME::delay_ms(ms3);    /// 10 msec 
        digitalWrite(b_pin.panelReset, HIGH);
        // comando reset
        TYME::delay_ms(ms4);    /// 5 msec 
        digitalWrite(b_pin.panelCS, HIGH); // CS# = 1
        TYME::delay_ms(ms5);    /// 5 msec 
    }

    void hV_Board_t::COG_SmallCJ_update(){
        // Application note § 5. Send updating command
        // Application note § 3.3 DC/DC soft-start
        // Application note § 4. Send updating command

        // Display Refresh Start
        b_waitBusy();
        b_sendCommand8(0x04); // Power on
        b_waitBusy();
        b_sendCommand8(0x12); // Display Refresh
        TYME::delay_ms(5);
        b_waitBusy();
    }

    void hV_Board_t::b_sendIndexFixedSelect(uint8_t index, uint8_t data, uint32_t size, uint8_t select)
    {
        digitalWrite(b_pin.panelDC, LOW); // DC Low = Command
        b_select(select); // Select half of large screen

        TYME::delay_us(b_delayCS);  // Longer delay for large screens
        hV_HAL_SPI_transfer(index);
        TYME::delay_us(b_delayCS);  // Longer delay for large screens

        digitalWrite(b_pin.panelDC, HIGH); // DC High = Data

        TYME::delay_us(b_delayCS);  // Longer delay for large screens
        for (uint32_t i = 0; i < size; i++)
        {
            hV_HAL_SPI_transfer(data); // b_sendIndexFixed
        }
        TYME::delay_us(b_delayCS); 

        digitalWrite(b_pin.panelCS, HIGH); // CS High = Unselect Master

    }


    void hV_Board_t::b_sendIndexData(uint8_t index, const uint8_t * data, uint32_t size)
    {

        digitalWrite(b_pin.panelDC, LOW); // DC Low
        digitalWrite(b_pin.panelCS, LOW); // CS Low

        TYME::delay_us(b_delayCS);
        hV_HAL_SPI_transfer(index);
        TYME::delay_us(b_delayCS);

        digitalWrite(b_pin.panelCS, HIGH); // CS High
        digitalWrite(b_pin.panelDC, HIGH); // DC High
        digitalWrite(b_pin.panelCS, LOW); // CS Low

        TYME::delay_us(b_delayCS);
        for (uint32_t i = 0; i < size; i++)
        {
            hV_HAL_SPI_transfer(data[i]);
        }
        TYME::delay_us(b_delayCS);
        digitalWrite(b_pin.panelCS, HIGH); // CS High
        TYME::delay_us(b_delayCS);
    }


    void hV_Board_t::b_begin( pins_t& board,  uint8_t family,  uint16_t delayCS)
    {
        //b_pin = board;
        b_family = family;
        b_delayCS = delayCS;
        b_fsmPowerScreen = FSM_OFF;
    }


    void hV_Board_t::COG_powerOff()
    {
    	//b_sendIndexData( 0x02, &register_data[0], 0 );  //Turn off DC/DC    
               b_sendIndexData( 0x02,0x00, 0 );  //Turn off DC/DC    
        while ( digitalRead( b_pin.panelBusy ) != std::stoi(HIGH));
    	digitalWrite( b_pin.panelDC, LOW );
    	digitalWrite( b_pin.panelCS, LOW );
    	digitalWrite( b_pin.panelBusy, LOW );
    	TYME::delay_ms( 150 );
    	digitalWrite( b_pin.panelReset, LOW );
    }


    void hV_Board_t::COG_SmallCJ_sendImageData()
    {
        // Application note § 4. Input image to the EPD
        FRAMEBUFFER_TYPE blackBuffer = s_newImage;
        FRAMEBUFFER_TYPE redBuffer = s_newImage + u_pageColourSize;

        // Send image data

        b_sendIndexData(0x10, blackBuffer, u_pageColourSize); // First frame
        b_sendIndexData(0x13, redBuffer, u_pageColourSize); // Second frame
                
        
    }


    void hV_Board_t::COG_SmallCJ_getDataOTP()
    {
        // Application note § none
        // Application note § 1.5 Read MTP data
        // Application note § 1.6 Read MTP data
        [[maybe_unused]] uint16_t _readBytes = 0;
        [[maybe_unused]] uint8_t ui8 = 0; // dummy
        u_flagOTP = false;

        _readBytes = 2;



                COG_data[0] = 0xcf;
                COG_data[1] = 0x8d;


        u_flagOTP = true;
    }


    void hV_Board_t::begin(){
        // u_eScreen_EPD = eScreen_EPD_EXT3;
        u_codeSize = SCREEN_SIZE(u_eScreen_EPD);
        u_codeFilm = SCREEN_FILM(u_eScreen_EPD);
        u_codeDriver = SCREEN_DRIVER(u_eScreen_EPD);
        u_codeExtra = SCREEN_EXTRA(u_eScreen_EPD);
        v_screenColourBits = 2; // BWR and BWRY

        // Checks
        switch (u_codeFilm)
        {
            case FILM_C: // Standard
            case FILM_H: // BW, Freezer
            case FILM_E: // BWR, deprecated
            case FILM_F: // BWR, deprecated
            case FILM_G: // BWY, deprecated
            case FILM_J: // BWR, "Spectra"

                break;

            default:

                debugVariant(FILM_C);
                break;
        }

        b_begin(b_pin, FAMILY_SMALL, 0);
    
        
    //        case SIZE_213: // 2.13"
    //        v_screenSizeV = 212; // vertical = wide size
    //        v_screenSizeH = 104; // horizontal = small size

                v_screenSizeV = 296; // vertical = wide size
                v_screenSizeH = 152; // horizontal = small size
    //          break;

    
        v_screenDiagonal = u_codeSize;

        // Report
        //mySerial.println(formatString("hV = Screen %s %ix%i", WhoAmI().c_str(), screenSizeX(), screenSizeY()));
        //mySerial.println(formatString("hV = Number %i-%cS-0%c", u_codeSize, u_codeFilm, u_codeDriver));
        //mySerial.println(formatString("hV = PDLS %s v%i.%i.%i", SCREEN_EPD_EXT3_VARIANT, SCREEN_EPD_EXT3_RELEASE / 100, (SCREEN_EPD_EXT3_RELEASE / 10) % 10, SCREEN_EPD_EXT3_RELEASE % 10));
        //mySerial.println();

        u_bufferDepth = v_screenColourBits; // 2 colours
        u_bufferSizeV = v_screenSizeV; // vertical = wide size
        u_bufferSizeH = v_screenSizeH / 8; // horizontal = small size 112 / 8, 1 bit per pixel

        // Force conversion for two unit16_t multiplication into uint32_t.
        // Actually for 1 colour; BWR requires 2 pages.
        u_pageColourSize = (uint32_t)u_bufferSizeV * (uint32_t)u_bufferSizeH;

        if (s_newImage == 0)
        {
            static uint8_t * _newFrameBuffer;
            _newFrameBuffer = new uint8_t[u_pageColourSize * u_bufferDepth];
            s_newImage = (uint8_t *) _newFrameBuffer;
        }



        memset(s_newImage, 0x00, u_pageColourSize * u_bufferDepth);

        setTemperatureC(25); // 25 Celsius = 77 Fahrenheit
        b_fsmPowerScreen = FSM_OFF;
        setPowerProfile(MODE_AUTO, SCOPE_GPIO_ONLY);

        // Turn SPI on, initialise GPIOs and set GPIO levels
        // Reset panel and get tables
        resume();

        // Fonts
        //hV_Screen_Buffer::begin(); // Standard

        if (f_fontMax() > 0)
        {
            f_selectFont(0);
        }
        f_fontSolid = false;

        // Orientation
        setOrientation(0);

        v_penSolid = false;
        u_invert = false;

    }


    void hV_Board_t::setTemperatureC(int8_t temperatureC)
    {
        u_temperature = temperatureC;

    }


    void hV_Board_t::resume()
    {
        // Target   FSM_ON
        // Source   FSM_OFF
        //          FSM_SLEEP
        if (b_fsmPowerScreen != FSM_ON)
        {
            if ((b_fsmPowerScreen & FSM_GPIO_MASK) != FSM_GPIO_MASK)
            {
                b_resume(); // GPIO

                COG_SmallCJ_reset();

                b_fsmPowerScreen |= FSM_GPIO_MASK;
            }

            // Check type and get tables
            if (u_flagOTP == false)
            {
                s_getDataOTP(); // 3-wire SPI read OTP memory

               COG_SmallCJ_reset();
            }

            // Start SPI
            //hV_HAL_SPI_begin(16000000); // Fast 16 MHz, with unicity check
        }
    }



    uint8_t hV_Board_t::f_fontMax()
    {
        return MAX_FONT_SIZE;
    }

    void hV_Board_t::s_getDataOTP()
    {
     //   hV_HAL_SPI_end(); // With unicity check

        //hV_HAL_SPI3_begin(); // Define 3-wire SPI pins
    

                COG_SmallCJ_getDataOTP();

    }


    void hV_Board_t::COG_SmallCJ_reset(){   
        
        b_reset(5, 5, 10, 5, 5);
    }

    void hV_Board_t::setPowerProfile(uint8_t mode, uint8_t scope)
    {
        u_suspendMode = mode;
        u_suspendScope = scope;
    
    }


void hV_Board_t::f_selectFont(uint8_t size)
{
    if (size < MAX_FONT_SIZE)
    {
        f_fontSize = size;
    }
    else
    {
        f_fontSize = MAX_FONT_SIZE - 1;
    }

    switch (f_fontSize)
    {
        case 0:
            // kind, height, maxWidth, first, number
            f_font = { 0x40, 8, 6, 32, 224 };
            break;

        case 1:
            f_font = { 0x40, 12, 8, 32, 224 };
            break;

        case 2:
            f_font = { 0x40, 16, 12, 32, 224 };
            break;

        case 3:
            f_font = { 0x40, 24, 16, 32, 224 };
            break;

        default:
            break;
    }
}


    void hV_Board_t::s_setOrientation(uint8_t orientation)
    {
        v_orientation = orientation % 4;
    }


uint16_t hV_Board_t::screenSizeX()
{
    switch (v_orientation)
    {
        case 1:
        case 3:

            return v_screenSizeV; // _maxX
            break;

        // case 0:
        // case 2:
        default:

            return v_screenSizeH; // _maxX
            break;
    }
    return 0;
}

    void hV_Board_t::setOrientation(uint8_t orientation)
    {
        switch (orientation)
        {
            case ORIENTATION_PORTRAIT:

                v_orientation = 0;
                s_setOrientation(v_orientation);
                if (screenSizeX() > screenSizeY())
                {
                    v_orientation += 1;
                    s_setOrientation(v_orientation);
                }
                break;

            case ORIENTATION_LANDSCAPE:

                v_orientation = 2;
                s_setOrientation(v_orientation);
                if (screenSizeX() < screenSizeY())
                {
                    v_orientation += 1;
                    s_setOrientation(v_orientation);
                }
                break;

            default:

                v_orientation = orientation % 4;
                s_setOrientation(v_orientation);
                break;
        }
    }



    uint16_t hV_Board_t::screenSizeY()
    {
        switch (v_orientation)
        {
            case 1:
            case 3:

                return v_screenSizeH; // _maxY
                break;

            // case 0:
            // case 2:
            default:

                return v_screenSizeV; // _maxY
                break;
        }
        return 0;
    }


    void hV_Board_t::s_flush(uint8_t updateMode)
    {
        // Resume
        if (b_fsmPowerScreen != FSM_ON)
        {
            resume();
        }

        // Three groups:

                COG_SmallCJ_initial(); // Initialise
                COG_SmallCJ_sendImageData(); // Send image data
                COG_SmallCJ_update(); // Update
                COG_SmallCJ_powerOff(); // Power 

        // Turn SPI off and pull GPIOs low
        //suspend();
    }


    void hV_Board_t::suspend(uint8_t suspendScope)
    {
        //if (((suspendScope & FSM_GPIO_MASK) == FSM_GPIO_MASK) and (b_pin.panelPower != NOT_CONNECTED))
        //{
        //    if ((b_fsmPowerScreen & FSM_GPIO_MASK) == FSM_GPIO_MASK)
        //    {
        //        b_suspend();
        //    }
        //}
    }

}