    #include <avr/interrupt.h>
    #include <LiquidCrystal.h>
    LiquidCrystal lcd(11, 10, 5, 4, 3, 2);

    #define LEDGREEN   7
    #define LEDRED     8
    #define BUTTON     6
    #define RELAY      13

    float voltage_of_bat, voltage_of_temsensor,voltage_of_ACS, adc_value1, adc_value2, adc_value3; 
    float current;
    int temperature, soc;
    float consumption = 0;
    float capacity = 10400;
    bool flag_ON = 1;
    bool flag_TOP = 1;

    void normalmode()
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("V: ");
      lcd.print(voltage_of_bat);
      lcd.print(" ");
      lcd.print("I: ");
      lcd.print(current);
      lcd.setCursor(0, 1);
      lcd.print("SOC: ");
      lcd.print(soc);
      lcd.print("% ");
      lcd.print("T: ");
      lcd.print(temperature);
    }

    void high_tem_mode()
    {
      lcd.setCursor(0, 1);
      lcd.print("HIGH TEMPERATURE");
    }

    void high_vol_mode()
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("HIGH VOLTAGE!!!");
    }

    void low_vol_mode()
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" LOW VOLTAGE!!!");
    }

    void low_soc()
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LOW SOC!!!");
    }

    //--------------------------------------
    void setup()
    {
      //SETUP LCD
      lcd.begin(16, 2);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   WELCOME TO  ");
      lcd.setCursor(0, 1);
      lcd.print("      BMS    ");
      delay(1500);
      
      Serial.begin(9600);
      pinMode(A0,INPUT);
      pinMode(A1, INPUT);
      pinMode(A2, INPUT);
      pinMode(BUTTON, INPUT);
      pinMode(LEDGREEN, OUTPUT);
      pinMode(LEDRED, OUTPUT);
      pinMode(RELAY, OUTPUT);
      cli();
      
      //Timer1
      TCCR1A = 0;
      TCCR1B = 0;
      TIMSK1 = 0;
      
      TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10); // prescale = 64 and CTC mode 4
      OCR1A = 9999;
      TIMSK1 = (1 << OCIE1A);     // Output Compare Interrupt Enable
  
      sei(); 
    }
    
    //-----------------------------------
    ISR(TIMER1_COMPA_vect)
    {
      if (flag_ON == 1)
      {
        flag_ON = !flag_ON;
  
        // Đọc giá trị nhiệt độ của PIN
        adc_value2 = analogRead(A0);
        temperature = map(adc_value2, 0, 1023, 0, 500);
  
        if (flag_TOP == 1)
        {
          flag_TOP = 0;
          // Đọc giá trị điện áp của PIN
          adc_value1 = analogRead(A1);
          voltage_of_bat = ((adc_value1 / 1024) * 100);

          // Đọc giá trị dòng điện thông qua IC ACS
          adc_value3 = analogRead(A2);
          voltage_of_ACS = map (adc_value3, 0, 1023, 2.5, 5.0);
          current = (voltage_of_ACS - 2.5) / 0.1455;
          if (temperature <= 60)
          {
            if (voltage_of_bat < 32.0)
            {
              digitalWrite(RELAY, LOW);
              low_vol_mode();
            } else if (voltage_of_bat > 45)
            {
              digitalWrite(RELAY, LOW);
              high_vol_mode();
            } else
            {
              digitalWrite(RELAY, HIGH);
              normalmode();
            }
          }
          else
          {
              if (voltage_of_bat < 32.0)
            {
              digitalWrite(RELAY, LOW);
              low_vol_mode();
              high_tem_mode();
              
            } else if (voltage_of_bat > 72)
            {
              digitalWrite(RELAY, LOW);
              high_vol_mode();
              high_tem_mode();
            } else
            {
              digitalWrite(RELAY, HIGH);
              lcd.clear();
              high_tem_mode();
            }
          }
        }
        
        // Tính toán trạng thái sạc (%SOC)
        consumption = consumption + (voltage_of_bat * current * 0.95) / 360.0;
        soc = (1.0 - (consumption / capacity)) * 100.0;
  
        //Báo SOC bằng đèn lED
        digitalWrite(LEDGREEN, LOW);
        digitalWrite(LEDRED, LOW);
        if (digitalRead(BUTTON) == 0)
        {
          if(soc > 20.0)
          {
            digitalWrite(LEDGREEN, HIGH);
            digitalWrite(LEDRED, LOW);
          } else
          {
            digitalWrite(LEDGREEN, LOW);
            digitalWrite(LEDRED, HIGH);
            low_soc();
          }
         }
        
        //Serial.print("Gia tri dien ap: ");
        Serial.println(voltage_of_bat);
        /*Serial.print("     ");
        Serial.print("Nhiet do cua pin: ");
        Serial.print(temperature);
        Serial.print("     ");
        Serial.print("Dong: ");
        Serial.print(current);
        Serial.print("     ");
        Serial.print("Consumption: ");
        Serial.print(consumption);
        Serial.print("     ");
        Serial.print("SOC: ");
        Serial.print(soc);
        Serial.println("%");
        */
        flag_TOP = 1;
        flag_ON = 1;
      }
    }
    
    
    //-------------------------------------------------
    
    void loop()
    {
      // add something
    }
