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
  static constexpr uint8_t MAX_PALETTES = 3;
  static constexpr uint8_t NUM_GEQ_CHANNELS = 16; // number of frequency channels. Don't change !!

  bool udpSyncConnected = false;         // UDP connection status -> true if connected to multicast group
  unsigned long last_connection_attempt = 0;
  
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

    static constexpr uint8_t  UDPSOUND_MAX_PACKET = 88;  // max packet size for audiosync

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

    float syncVolumeSmth = 0;

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
    /// @see Usermod::getId()
    /// @see MyExampleUsermod::getId()
    uint16_t getId() override
    {
      // we are pretending to be usermod AudioReactive so that the effects
      // won't notice a difference where the audio data is comming from
      return USERMOD_ID_AUDIOREACTIVE;
    }


    /// @see Usermod::getUMData()
    /// @see MyExampleUsermod::getUMData()
    bool getUMData(um_data_t **data) override
    {
      if (!enabled || !data) return false;
      *data = um_data;
      return true;
    }


    /// @see Usermod::setup()
    /// @see MyExampleUsermod::setup()
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


    /// @see Usermod::connected()
    /// @see MyExampleUsermod::connected()
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


    /// @see Usermod::loop()
    /// @see MyExampleUsermod::loop()
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


    /// @see Usermod::onUpdateBegin()
    /// @see MyExampleUsermod::onUpdateBegin()
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


    /// @see Usermod::addToJsonInfo()
    /// @see MyExampleUsermod::addToJsonInfo()
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
            infoArr.add(F("receiving v"));
            if (receivedFormat == 1) infoArr.add(F("1"));
            if (receivedFormat == 2) infoArr.add(F("2"));
            }
          else
            infoArr.add(F("<i>(no server)</i>"));
        } else {
          infoArr.add(F("<i>(unconnected)</i>"));
        }
      }
    }


    /// @see Usermod::addToJsonState()
    /// @see MyExampleUsermod::addToJsonState()
    void addToJsonState(JsonObject& root) override
    {
      if (!isInitDone()) return;  // prevent crash on boot applyPreset()
      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) {
        usermod = root.createNestedObject(FPSTR(_name));
      }
      usermod["on"] = enabled;
    }


    /// @see Usermod::readFromJsonState()
    /// @see MyExampleUsermod::readFromJsonState()
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


    /// @see Usermod::onStateChange()
    /// @see MyExampleUsermod::onStateChange()
    void onStateChange(uint8_t callMode) override {
      if (isInitDone() && enabled && addPalettes && palettes==0 && strip.customPalettes.size()<10) {
        // if palettes were removed during JSON call re-add them
        createAudioPalettes();
      }
    }


    /// @see Usermod::addToConfig()
    /// @see MyExampleUsermod::addToConfig()
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


    /// @see Usermod::readFromConfig()
    /// @see MyExampleUsermod::readFromConfig()
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


    /// @see Usermod::appendConfigData()
    /// @see MyExampleUsermod::appendConfigData()
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

// strings to reduce flash memory usage (used more than twice)
const char UmAudioReceiver::_name[]       PROGMEM = "AudioReceiver";
const char UmAudioReceiver::_enabled[]    PROGMEM = "enabled";
const char UmAudioReceiver::_dynamics[]   PROGMEM = "dynamics";
const char UmAudioReceiver::_addPalettes[]       PROGMEM = "add-palettes";
const char UmAudioReceiver::UDP_SYNC_HEADER[]    PROGMEM = "00002"; // new sync header version, as format no longer compatible with previous structure
const char UmAudioReceiver::UDP_SYNC_HEADER_v1[] PROGMEM = "00001"; // old sync header version - need to add backwards-compatibility feature

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
