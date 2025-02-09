/**
 * Usermod v2: AudioReceiver
 *
 * Usermods allow you to add own functionality to WLED more easily
 * @see https://github.com/Aircoookie/WLED/tree/main/usermods/EXAMPLE_v2
 */

#pragma once

#include "wled.h"

#ifdef USERMOD_AUDIOREACTIVE
#error "This usermod AudioReceiver cannot be used together with usermod AudioReactive!"
#endif

//--------------------------------------------------------------------------------------------------

/** Completely stripped down clone of AudioReactive, where only the UDP receiver part is left.
 * This usermod is intended to make audioreactive WLED effects also usable on limited targets, like
 * the ESP32-C3 which lacks a floating point unit. The original AudioReactive usermod could run
 * there as well, but the performance would be very poor. So instead of AudioReactive, just enable
 * this lightweight AudioReceiver as alternative. \n
 * To use this usermod, you also need a server, which is transmitting the preprocessed audio
 * data over UDP. This can be another WLED instancce with a real AudioReactive usermod running, or
 * a Windows server like https://github.com/Victoare/SR-WLED-audio-server-win
 */
class UmAudioReceiver : public Usermod {

  #define MAX_PALETTES 3

  bool udpSyncConnected = false;         // UDP connection status -> true if connected to multicast group
  
  #define NUM_GEQ_CHANNELS 16                                           // number of frequency channels. Don't change !!
  
  // audioreactive variables
  // variables used in effects
  float   volumeSmth = 0.0f;    // either sampleAvg or sampleAgc depending on soundAgc; smoothed sample
  int16_t  volumeRaw = 0;       // either sampleRaw or rawSampleAgc depending on soundAgc
  float my_magnitude =0.0f;     // FFT_Magnitude, scaled by multAgc
  float FFT_MajorPeak = 1.0f;              // FFT: strongest (peak) frequency
  float FFT_Magnitude = 0.0f;              // FFT: volume (magnitude) of peak frequency
  bool samplePeak = false;      // Boolean flag for peak - used in effects. Responding routine may reset this flag. Auto-reset after strip.getFrameTime()
  uint8_t fftResult[NUM_GEQ_CHANNELS]= {0};// Our calculated freq. channel result table to be used by effects
  
  // user settable parameters for limitSoundDynamics()
  #ifdef UM_AUDIOREACTIVE_DYNAMICS_LIMITER_OFF
  bool limiterOn = false;                 // bool: enable / disable dynamics limiter
  #else
  bool limiterOn = true;
  #endif
  uint16_t attackTime =  80;             // int: attack time in milliseconds. Default 0.08sec
  uint16_t decayTime = 1400;             // int: decay time in milliseconds.  Default 1.40sec
  
  // peak detection
  uint8_t maxVol = 31;          // (was 10) Reasonable value for constant volume for 'peak detector', as it won't always trigger  (deprecated)
  uint8_t binNum = 8;           // Used to select the bin for FFT based beat detection  (deprecated)
  
  unsigned long timeOfPeak = 0; // time of last sample peak detection.

  void autoResetPeak() {
    uint16_t peakDelay = max(uint16_t(50), strip.getFrameTime());
    if (millis() - timeOfPeak > peakDelay) {          // Auto-reset of samplePeak after at least one complete frame has passed.
      samplePeak = false;
    }
  }

    // new "V2" audiosync struct - 44 Bytes
    struct __attribute__ ((packed)) audioSyncPacket {  // "packed" ensures that there are no additional gaps
      char    header[6];      //  06 Bytes  offset 0
      uint8_t reserved1[2];   //  02 Bytes, offset 6  - gap required by the compiler - not used yet 
      float   sampleRaw;      //  04 Bytes  offset 8  - either "sampleRaw" or "rawSampleAgc" depending on soundAgc setting
      float   sampleSmth;     //  04 Bytes  offset 12 - either "sampleAvg" or "sampleAgc" depending on soundAgc setting
      uint8_t samplePeak;     //  01 Bytes  offset 16 - 0 no peak; >=1 peak detected. In future, this will also provide peak Magnitude
      uint8_t reserved2;      //  01 Bytes  offset 17 - for future extensions - not used yet
      uint8_t fftResult[16];  //  16 Bytes  offset 18
      uint16_t reserved3;     //  02 Bytes, offset 34 - gap required by the compiler - not used yet
      float  FFT_Magnitude;   //  04 Bytes  offset 36
      float  FFT_MajorPeak;   //  04 Bytes  offset 40
    };

    // old "V1" audiosync struct - 83 Bytes payload, 88 bytes total (with padding added by compiler) - for backwards compatibility
    struct audioSyncPacket_v1 {
      char header[6];         //  06 Bytes
      uint8_t myVals[32];     //  32 Bytes
      int sampleAgc;          //  04 Bytes
      int sampleRaw;          //  04 Bytes
      float sampleAvg;        //  04 Bytes
      bool samplePeak;        //  01 Bytes
      uint8_t fftResult[16];  //  16 Bytes
      double FFT_Magnitude;   //  08 Bytes
      double FFT_MajorPeak;   //  08 Bytes
    };

    #define UDPSOUND_MAX_PACKET 88 // max packet size for audiosync

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    #ifdef UM_AUDIOREACTIVE_ENABLE
    bool     enabled = true;
    #else
    bool     enabled = false;
    #endif

    bool isInitDone() const { return um_data != nullptr; }

    bool     addPalettes = false;
    int8_t   palettes = 0;

    // variables  for UDP sound sync
    WiFiUDP fftUdp;               // UDP object for sound sync (from WiFi UDP, not Async UDP!) 
    unsigned long lastTime = 0;   // last time of running UDP Microphone Sync
    const uint16_t delayMs = 10;  // I don't want to sample too often and overload WLED
    uint16_t audioSyncPort= 11988;// default port for UDP sound sync

    bool updateIsRunning = false; // true during OTA.

    // used to feed "Info" Page
    unsigned long last_UDPTime = 0;    // time of last valid UDP sound sync datapacket
    int receivedFormat = 0;            // last received UDP sound sync format - 0=none, 1=v1 (0.13.x), 2=v2 (0.14.x)

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _dynamics[];
    static const char _addPalettes[];
    static const char UDP_SYNC_HEADER[];
    static const char UDP_SYNC_HEADER_v1[];

    // private methods
    void removeAudioPalettes(void);
    void createAudioPalettes(void);
    CRGB getCRGBForBand(int x, int pal);
    void fillAudioPalettes(void);

    unsigned long last_time = 0;
    float last_volumeSmth = 0.0f;
  /* Limits the dynamics of volumeSmth (= sampleAvg or sampleAgc). 
     * does not affect FFTResult[] or volumeRaw ( = sample or rawSampleAgc) 
    */
    // effects: Gravimeter, Gravcenter, Gravcentric, Noisefire, Plasmoid, Freqpixels, Freqwave, Gravfreq, (2D Swirl, 2D Waverly)
    void limitSampleDynamics(void) {
      const float bigChange = 196;                  // just a representative number - a large, expected sample value

      if (limiterOn == false) return;

      long delta_time = millis() - last_time;
      delta_time = constrain(delta_time , 1, 1000); // below 1ms -> 1ms; above 1sec -> sily lil hick-up
      float deltaSample = volumeSmth - last_volumeSmth;

      if (attackTime > 0) {                         // user has defined attack time > 0
        float maxAttack =   bigChange * float(delta_time) / float(attackTime);
        if (deltaSample > maxAttack) deltaSample = maxAttack;
      }
      if (decayTime > 0) {                          // user has defined decay time > 0
        float maxDecay  = - bigChange * float(delta_time) / float(decayTime);
        if (deltaSample < maxDecay) deltaSample = maxDecay;
      }

      volumeSmth = last_volumeSmth + deltaSample; 

      last_volumeSmth = volumeSmth;
      last_time = millis();
    }

    unsigned long last_connection_attempt = 0;

    // try to establish UDP sound sync connection
    void connectUDPSoundSync(void) {
      // This function tries to establish a UDP sync connection if needed
      // necessary as we also want to transmit in "AP Mode", but the standard "connected()" callback only reacts on STA connection

      if (audioSyncPort <= 0) return;
      if (udpSyncConnected) return;                                          // already connected
      if (!(apActive || interfacesInited)) return;                           // neither AP nor other connections availeable
      if (millis() - last_connection_attempt < 15000) return;                // only try once in 15 seconds
      if (updateIsRunning) return; 

      // if we arrive here, we need a UDP connection but don't have one
      last_connection_attempt = millis();
      connected(); // try to start UDP
    }

    static bool isValidUdpSyncVersion(const char *header) {
      return strncmp_P(header, UDP_SYNC_HEADER, 6) == 0;
    }
    static bool isValidUdpSyncVersion_v1(const char *header) {
      return strncmp_P(header, UDP_SYNC_HEADER_v1, 6) == 0;
    }

    void decodeAudioData(int packetSize, uint8_t *fftBuff) {
      audioSyncPacket receivedPacket;
      memset(&receivedPacket, 0, sizeof(receivedPacket));                                  // start clean
      memcpy(&receivedPacket, fftBuff, min((unsigned)packetSize, (unsigned)sizeof(receivedPacket))); // don't violate alignment - thanks @willmmiles#

      // update samples for effects
      volumeSmth   = fmaxf(receivedPacket.sampleSmth, 0.0f);
      volumeRaw    = fmaxf(receivedPacket.sampleRaw, 0.0f);
      // Only change samplePeak IF it's currently false.
      // If it's true already, then the animation still needs to respond.
      autoResetPeak();
      if (!samplePeak) {
            samplePeak = receivedPacket.samplePeak >0 ? true:false;
            if (samplePeak) timeOfPeak = millis();
      }
      //These values are only computed by ESP32
      for (int i = 0; i < NUM_GEQ_CHANNELS; i++) fftResult[i] = receivedPacket.fftResult[i];
      my_magnitude  = fmaxf(receivedPacket.FFT_Magnitude, 0.0f);
      FFT_Magnitude = my_magnitude;
      FFT_MajorPeak = constrain(receivedPacket.FFT_MajorPeak, 1.0f, 11025.0f);  // restrict value to range expected by effects
    }

    void decodeAudioData_v1(int packetSize, uint8_t *fftBuff) {
      audioSyncPacket_v1 *receivedPacket = reinterpret_cast<audioSyncPacket_v1*>(fftBuff);
      // update samples for effects
      volumeSmth   = fmaxf(receivedPacket->sampleAgc, 0.0f);
      volumeRaw    = volumeSmth;   // V1 format does not have "raw" AGC sample
      // Only change samplePeak IF it's currently false.
      // If it's true already, then the animation still needs to respond.
      autoResetPeak();
      if (!samplePeak) {
            samplePeak = receivedPacket->samplePeak >0 ? true:false;
            if (samplePeak) timeOfPeak = millis();
      }
      //These values are only available on the ESP32
      for (int i = 0; i < NUM_GEQ_CHANNELS; i++) fftResult[i] = receivedPacket->fftResult[i];
      my_magnitude  = fmaxf(receivedPacket->FFT_Magnitude, 0.0);
      FFT_Magnitude = my_magnitude;
      FFT_MajorPeak = constrain(receivedPacket->FFT_MajorPeak, 1.0, 11025.0);  // restrict value to range expected by effects
    }

    bool receiveAudioData()   // check & process new data. return TRUE in case that new audio data was received. 
    {
      if (!udpSyncConnected) return false;
      bool haveFreshData = false;

      size_t packetSize = fftUdp.parsePacket();
#ifdef ARDUINO_ARCH_ESP32
      if ((packetSize > 0) && ((packetSize < 5) || (packetSize > UDPSOUND_MAX_PACKET))) fftUdp.flush(); // discard invalid packets (too small or too big) - only works on esp32
#endif
      if ((packetSize > 5) && (packetSize <= UDPSOUND_MAX_PACKET)) {
        uint8_t fftBuff[UDPSOUND_MAX_PACKET+1] = { 0 }; // fixed-size buffer for receiving (stack), to avoid heap fragmentation caused by variable sized arrays
        fftUdp.read(fftBuff, packetSize);

        // VERIFY THAT THIS IS A COMPATIBLE PACKET
        if (packetSize == sizeof(audioSyncPacket) && (isValidUdpSyncVersion((const char *)fftBuff))) {
          decodeAudioData(packetSize, fftBuff);
          haveFreshData = true;
          receivedFormat = 2;
        } else {
          if (packetSize == sizeof(audioSyncPacket_v1) && (isValidUdpSyncVersion_v1((const char *)fftBuff))) {
            decodeAudioData_v1(packetSize, fftBuff);
            haveFreshData = true;
            receivedFormat = 1;
          } else receivedFormat = 0; // unknown format
        }
      }
      return haveFreshData;
    }


  public:
    // we are pretending to be usermod AudioReactive so that the effects
    // won't notice a difference where the audio data is comming from
    uint16_t getId() override { return USERMOD_ID_AUDIOREACTIVE; }

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     * It is called *AFTER* readFromConfig()
     */
    void setup() override
    {
      if (!isInitDone()) {
        // usermod exchangeable data
        // we will assign all usermod exportable data here as pointers to original variables or arrays and allocate memory for pointers
        um_data = new um_data_t;
        um_data->u_size = 8;
        um_data->u_type = new um_types_t[um_data->u_size];
        um_data->u_data = new void*[um_data->u_size];
        um_data->u_data[0] = &volumeSmth;      //*used (New)
        um_data->u_type[0] = UMT_FLOAT;
        um_data->u_data[1] = &volumeRaw;      // used (New)
        um_data->u_type[1] = UMT_UINT16;
        um_data->u_data[2] = fftResult;        //*used (Blurz, DJ Light, Noisemove, GEQ_base, 2D Funky Plank, Akemi)
        um_data->u_type[2] = UMT_BYTE_ARR;
        um_data->u_data[3] = &samplePeak;      //*used (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[3] = UMT_BYTE;
        um_data->u_data[4] = &FFT_MajorPeak;   //*used (Ripplepeak, Freqmap, Freqmatrix, Freqpixels, Freqwave, Gravfreq, Rocktaves, Waterfall)
        um_data->u_type[4] = UMT_FLOAT;
        um_data->u_data[5] = &my_magnitude;   // used (New)
        um_data->u_type[5] = UMT_FLOAT;
        um_data->u_data[6] = &maxVol;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[6] = UMT_BYTE;
        um_data->u_data[7] = &binNum;          // assigned in effect function from UI element!!! (Puddlepeak, Ripplepeak, Waterfall)
        um_data->u_type[7] = UMT_BYTE;
      }

      if (enabled) onUpdateBegin(false);                 // initialize network
      if (enabled) connectUDPSoundSync();
      if (enabled && addPalettes) createAudioPalettes();
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() override
    {
      if (udpSyncConnected) {   // clean-up: if open, close old UDP sync connection
        udpSyncConnected = false;
        fftUdp.stop();
      }
      
      if (audioSyncPort > 0) {
      #ifdef ARDUINO_ARCH_ESP32
        udpSyncConnected = fftUdp.beginMulticast(IPAddress(239, 0, 0, 1), audioSyncPort);
      #else
        udpSyncConnected = fftUdp.beginMulticast(WiFi.localIP(), IPAddress(239, 0, 0, 1), audioSyncPort);
      #endif
      }
    }


    float syncVolumeSmth = 0;
    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() override
    {
      if (!enabled) {
        return;
      }

      autoResetPeak();          // auto-reset sample peak after strip minShowDelay

      connectUDPSoundSync();  // ensure we have a connection - if needed

      if (udpSyncConnected) {
          bool have_new_sample = false;
          if (millis() - lastTime > delayMs) {
            have_new_sample = receiveAudioData();
            if (have_new_sample) last_UDPTime = millis();
#ifdef ARDUINO_ARCH_ESP32
            else fftUdp.flush(); // Flush udp input buffers if we haven't read it - avoids hickups in receive mode. Does not work on 8266.
#endif
            lastTime = millis();
          }
          if (have_new_sample) syncVolumeSmth = volumeSmth;   // remember received sample
          else volumeSmth = syncVolumeSmth;                   // restore originally received sample for next run of dynamics limiter
          limitSampleDynamics();                              // run dynamics limiter on received volumeSmth, to hide jumps and hickups
      }

      fillAudioPalettes();
    }


    bool getUMData(um_data_t **data) override
    {
      if (!data || !enabled) return false; // no pointer provided by caller or not enabled -> exit
      *data = um_data;
      return true;
    }


    void onUpdateBegin(bool init) override
    {
      // reset sound data
      volumeRaw = 0; volumeSmth = 0;
      my_magnitude = 0; FFT_Magnitude = 0; FFT_MajorPeak = 1;
      memset(fftResult, 0, sizeof(fftResult)); 
      for(int i=(init?0:1); i<NUM_GEQ_CHANNELS; i+=2) fftResult[i] = 16; // make a tiny pattern
      // samplePeak = false;
      autoResetPeak();
      if (init) {
        if (udpSyncConnected) {   // close UDP sync connection (if open)
          udpSyncConnected = false;
          fftUdp.stop();
          receivedFormat = 0;
        }
      }
      updateIsRunning = init;
    }

    ////////////////////////////
    // Settings and Info Page //
    ////////////////////////////

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) override
    {
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray infoArr = user.createNestedArray(FPSTR(_name));

      String uiDomString = F("<button class=\"btn btn-xs\" onclick=\"requestJson({");
      uiDomString += FPSTR(_name);
      uiDomString += F(":{");
      uiDomString += FPSTR(_enabled);
      uiDomString += enabled ? F(":false}});\">") : F(":true}});\">");
      uiDomString += F("<i class=\"icons");
      uiDomString += enabled ? F(" on") : F(" off");
      uiDomString += F("\">&#xe08f;</i>");
      uiDomString += F("</button>");
      infoArr.add(uiDomString);

      if (enabled) {
        infoArr = user.createNestedArray(F("UDP Sound Sync"));
        if (udpSyncConnected) {
          if (millis() - last_UDPTime < 2500)
          {
            infoArr.add(F("receiving"));
            if (receivedFormat == 1) infoArr.add(F(" v1"));
            if (receivedFormat == 2) infoArr.add(F(" v2"));
            }
          else
            infoArr.add(F("idle"));
        } else {
          infoArr.add(F("<i>(unconnected)</i>"));
        }
      }
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root) override
    {
      if (!isInitDone()) return;  // prevent crash on boot applyPreset()
      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) {
        usermod = root.createNestedObject(FPSTR(_name));
      }
      usermod["on"] = enabled;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) override
    {
      if (!isInitDone()) return;  // prevent crash on boot applyPreset()
      bool prevEnabled = enabled;
      JsonObject usermod = root[FPSTR(_name)];
      if (!usermod.isNull()) {
        if (usermod[FPSTR(_enabled)].is<bool>()) {
          enabled = usermod[FPSTR(_enabled)].as<bool>();
          if (prevEnabled != enabled) onUpdateBegin(!enabled);
          if (addPalettes) {
            // add/remove custom/audioreactive palettes
            if (prevEnabled && !enabled) removeAudioPalettes();
            if (!prevEnabled && enabled) createAudioPalettes();
          }
        }
      }
      if (root.containsKey(F("rmcpal")) && root[F("rmcpal")].as<bool>()) {
        // handle removal of custom palettes from JSON call so we don't break things
        removeAudioPalettes();
      }
    }

    void onStateChange(uint8_t callMode) override {
      if (isInitDone() && enabled && addPalettes && palettes==0 && strip.customPalettes.size()<10) {
        // if palettes were removed during JSON call re-add them
        createAudioPalettes();
      }
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) override
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
      top[FPSTR(_addPalettes)] = addPalettes;
      
      JsonObject udp = top.createNestedObject("UDP");
      udp["port"] = audioSyncPort;

      JsonObject dynLim = top.createNestedObject(FPSTR(_dynamics));
      dynLim[F("limiter")] = limiterOn;
      dynLim[F("rise")] = attackTime;
      dynLim[F("fall")] = decayTime;
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root) override
    {
      JsonObject top = root[FPSTR(_name)];
      bool configComplete = !top.isNull();
      bool oldEnabled = enabled;
      bool oldAddPalettes = addPalettes;

      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);
      configComplete &= getJsonValue(top[FPSTR(_addPalettes)], addPalettes);
      configComplete &= getJsonValue(top["UDP"]["port"], audioSyncPort);

      configComplete &= getJsonValue(top[FPSTR(_dynamics)][F("limiter")], limiterOn);
      configComplete &= getJsonValue(top[FPSTR(_dynamics)][F("rise")],  attackTime);
      configComplete &= getJsonValue(top[FPSTR(_dynamics)][F("fall")],  decayTime);

      if (isInitDone()) {
        // add/remove custom/audioreactive palettes
        if ((oldAddPalettes && !addPalettes) || (oldAddPalettes && !enabled)) removeAudioPalettes();
        if ((addPalettes && !oldAddPalettes && enabled) || (addPalettes && !oldEnabled && enabled)) createAudioPalettes();
      } // else setup() will create palettes
      return configComplete;
    }

    /*
     * appendConfigData() is called when user enters usermod settings page
     * it may add additional metadata for certain entry fields (adding drop down is possible)
     * be careful not to add too much as oappend() buffer is limited to 3k
     */
    void appendConfigData(Print& uiScript) override
    {
      uiScript.print(F("ux='AudioReceiver';"));         // ux = shortcut for AudioReceiver - fingers crossed that "ux" isn't already used as JS var, html post parameter or css style
      uiScript.print(F("dd=addDropdown(ux,'dynamics:limiter');"));
      uiScript.print(F("addOption(dd,'Off',0);"));
      uiScript.print(F("addOption(dd,'On',1);"));
      uiScript.print(F("addInfo(ux+':dynamics:limiter',0,' On ');"));  // 0 is field type, 1 is actual field
      uiScript.print(F("addInfo(ux+':dynamics:rise',1,'ms <i>(&#x266A; effects only)</i>');"));
      uiScript.print(F("addInfo(ux+':dynamics:fall',1,'ms <i>(&#x266A; effects only)</i>');"));
    }
};

//--------------------------------------------------------------------------------------------------

void UmAudioReceiver::removeAudioPalettes(void) {
  DEBUG_PRINTLN(F("Removing audio palettes."));
  while (palettes>0) {
    strip.customPalettes.pop_back();
    DEBUG_PRINTLN(palettes);
    palettes--;
  }
  DEBUG_PRINT(F("Total # of palettes: ")); DEBUG_PRINTLN(strip.customPalettes.size());
}

void UmAudioReceiver::createAudioPalettes(void) {
  DEBUG_PRINT(F("Total # of palettes: ")); DEBUG_PRINTLN(strip.customPalettes.size());
  if (palettes) return;
  DEBUG_PRINTLN(F("Adding audio palettes."));
  for (int i=0; i<MAX_PALETTES; i++)
    if (strip.customPalettes.size() < 10) {
      strip.customPalettes.push_back(CRGBPalette16(CRGB(BLACK)));
      palettes++;
      DEBUG_PRINTLN(palettes);
    } else break;
}

// credit @netmindz ar palette, adapted for usermod @blazoncek
CRGB UmAudioReceiver::getCRGBForBand(int x, int pal) {
  CRGB value;
  CHSV hsv;
  int b;
  switch (pal) {
    case 2:
      b = map(x, 0, 255, 0, NUM_GEQ_CHANNELS/2); // convert palette position to lower half of freq band
      hsv = CHSV(fftResult[b], 255, x);
      hsv2rgb_rainbow(hsv, value);  // convert to R,G,B
      break;
    case 1:
      b = map(x, 1, 255, 0, 10); // convert palette position to lower half of freq band
      hsv = CHSV(fftResult[b], 255, map(fftResult[b], 0, 255, 30, 255));  // pick hue
      hsv2rgb_rainbow(hsv, value);  // convert to R,G,B
      break;
    default:
      if (x == 1) {
        value = CRGB(fftResult[10]/2, fftResult[4]/2, fftResult[0]/2);
      } else if(x == 255) {
        value = CRGB(fftResult[10]/2, fftResult[0]/2, fftResult[4]/2);
      } else {
        value = CRGB(fftResult[0]/2, fftResult[4]/2, fftResult[10]/2);
      }
      break;
  }
  return value;
}

void UmAudioReceiver::fillAudioPalettes() {
  if (!palettes) return;
  size_t lastCustPalette = strip.customPalettes.size();
  if (int(lastCustPalette) >= palettes) lastCustPalette -= palettes;
  for (int pal=0; pal<palettes; pal++) {
    uint8_t tcp[16];  // Needs to be 4 times however many colors are being used.
                      // 3 colors = 12, 4 colors = 16, etc.

    tcp[0] = 0;  // anchor of first color - must be zero
    tcp[1] = 0;
    tcp[2] = 0;
    tcp[3] = 0;
    
    CRGB rgb = getCRGBForBand(1, pal);
    tcp[4] = 1;  // anchor of first color
    tcp[5] = rgb.r;
    tcp[6] = rgb.g;
    tcp[7] = rgb.b;
    
    rgb = getCRGBForBand(128, pal);
    tcp[8] = 128;
    tcp[9] = rgb.r;
    tcp[10] = rgb.g;
    tcp[11] = rgb.b;
    
    rgb = getCRGBForBand(255, pal);
    tcp[12] = 255;  // anchor of last color - must be 255
    tcp[13] = rgb.r;
    tcp[14] = rgb.g;
    tcp[15] = rgb.b;

    strip.customPalettes[lastCustPalette+pal].loadDynamicGradientPalette(tcp);
  }
}

// strings to reduce flash memory usage (used more than twice)
const char UmAudioReceiver::_name[]       PROGMEM = "AudioReceiver";
const char UmAudioReceiver::_enabled[]    PROGMEM = "enabled";
const char UmAudioReceiver::_dynamics[]   PROGMEM = "dynamics";
const char UmAudioReceiver::_addPalettes[]       PROGMEM = "add-palettes";
const char UmAudioReceiver::UDP_SYNC_HEADER[]    PROGMEM = "00002"; // new sync header version, as format no longer compatible with previous structure
const char UmAudioReceiver::UDP_SYNC_HEADER_v1[] PROGMEM = "00001"; // old sync header version - need to add backwards-compatibility feature
