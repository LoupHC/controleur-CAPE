//Assign same pin to different outputs
#if ROLLUP1_OPENING_PIN != OFF && (ROLLUP1_OPENING_PIN == ROLLUP1_CLOSING_PIN ||ROLLUP1_OPENING_PIN == ROLLUP2_CLOSING_PIN || ROLLUP1_OPENING_PIN == ROLLUP2_OPENING_PIN || ROLLUP1_OPENING_PIN == FAN1_PIN || ROLLUP1_OPENING_PIN == FAN2_PIN || ROLLUP1_OPENING_PIN == HEATER1_PIN || ROLLUP1_OPENING_PIN == HEATER2_PIN || ROLLUP1_OPENING_PIN == ALARM_PIN)
  #error Rollup1 opening pin attributed to other output
#endif

#if ROLLUP1_CLOSING_PIN != OFF  && (ROLLUP1_CLOSING_PIN == ROLLUP1_OPENING_PIN ||ROLLUP1_CLOSING_PIN == ROLLUP2_CLOSING_PIN || ROLLUP1_CLOSING_PIN == ROLLUP2_OPENING_PIN || ROLLUP1_CLOSING_PIN == FAN1_PIN || ROLLUP1_CLOSING_PIN == FAN2_PIN || ROLLUP1_CLOSING_PIN == HEATER1_PIN || ROLLUP1_CLOSING_PIN == HEATER2_PIN || ROLLUP1_CLOSING_PIN == ALARM_PIN)
  #error Rollup1 closing pin attributed to other output
#endif

#if ROLLUP2_OPENING_PIN != OFF && (ROLLUP2_OPENING_PIN == ROLLUP1_CLOSING_PIN ||ROLLUP2_OPENING_PIN == ROLLUP2_CLOSING_PIN || ROLLUP2_OPENING_PIN == ROLLUP1_OPENING_PIN || ROLLUP2_OPENING_PIN == FAN1_PIN || ROLLUP2_OPENING_PIN == FAN2_PIN || ROLLUP2_OPENING_PIN == HEATER1_PIN || ROLLUP2_OPENING_PIN == HEATER2_PIN || ROLLUP2_OPENING_PIN == ALARM_PIN)
  #error Rollup2 opening pin attributed to other output
#endif

#if ROLLUP2_CLOSING_PIN != OFF  && (ROLLUP2_CLOSING_PIN == ROLLUP1_CLOSING_PIN ||ROLLUP2_CLOSING_PIN == ROLLUP1_OPENING_PIN || ROLLUP2_CLOSING_PIN == ROLLUP2_OPENING_PIN || ROLLUP2_CLOSING_PIN == FAN1_PIN || ROLLUP2_CLOSING_PIN == FAN2_PIN || ROLLUP2_CLOSING_PIN == HEATER1_PIN || ROLLUP2_CLOSING_PIN == HEATER2_PIN || ROLLUP2_CLOSING_PIN == ALARM_PIN)
  #error Rollup2 closing pin attributed to other output
#endif

#if FAN1_PIN != OFF  && (FAN1_PIN == ROLLUP1_CLOSING_PIN ||FAN1_PIN == ROLLUP2_CLOSING_PIN || FAN1_PIN == ROLLUP2_OPENING_PIN || FAN1_PIN == ROLLUP1_OPENING_PIN || FAN1_PIN == FAN2_PIN || FAN1_PIN == HEATER1_PIN || FAN1_PIN == HEATER2_PIN || FAN1_PIN == ALARM_PIN)
  #error Fan1 pin attributed to other output
#endif

#if FAN2_PIN != OFF && (FAN2_PIN == ROLLUP1_CLOSING_PIN ||FAN2_PIN == ROLLUP2_CLOSING_PIN || FAN2_PIN == ROLLUP2_OPENING_PIN || FAN2_PIN == FAN1_PIN || FAN2_PIN == ROLLUP1_OPENING_PIN || FAN2_PIN == HEATER1_PIN || FAN2_PIN == HEATER2_PIN || FAN2_PIN == ALARM_PIN)
  #error Fan2 pin attributed to other output
#endif

#if HEATER1_PIN != OFF  && (HEATER1_PIN == ROLLUP1_CLOSING_PIN ||HEATER1_PIN == ROLLUP2_CLOSING_PIN || HEATER1_PIN == ROLLUP2_OPENING_PIN || HEATER1_PIN == FAN1_PIN || HEATER1_PIN == FAN2_PIN || HEATER1_PIN == ROLLUP1_OPENING_PIN || HEATER1_PIN == HEATER2_PIN || HEATER1_PIN == ALARM_PIN)
  #error Heater1 pin attributed to other output
#endif

#if HEATER2_PIN != OFF && (HEATER2_PIN == ROLLUP1_CLOSING_PIN ||HEATER2_PIN == ROLLUP2_CLOSING_PIN || HEATER2_PIN == ROLLUP2_OPENING_PIN || HEATER2_PIN == FAN1_PIN || HEATER2_PIN == FAN2_PIN || HEATER2_PIN == HEATER1_PIN || HEATER2_PIN == ROLLUP1_OPENING_PIN || HEATER2_PIN == ALARM_PIN)
  #error Heater2 pin attributed to other output
#endif


//Undeclared output pins
#if (ROLLUP1_OPENING_PIN == OFF || ROLLUP1_CLOSING_PIN == OFF) && ROLLUPS > 0
    #error Rollup1 pin is missing
#endif

#if (ROLLUP2_OPENING_PIN == OFF || ROLLUP2_CLOSING_PIN == OFF) && ROLLUPS > 1
      #error Rollup2 pin is missing
#endif

#if FAN1_PIN == OFF && FANS > 0
      #error Fan1 pin is missing
#endif

#if FAN2_PIN == OFF && FANS > 1
      #error Fan2 pin is missing
#endif

#if HEATER1_PIN == OFF && HEATERS > 0
      #error Heater1 pin is missing
#endif

#if HEATER2_PIN == OFF && HEATERS > 1
      #error Heater2 pin is missing
#endif

//Too much class instances for greenhouseLib
#if TIMEPOINTS < 0 ||  TIMEPOINTS > MAX_TIMEPOINTS
  #error So many timepoints... really?
#endif

#if ROLLUPS < 0 || ROLLUPS > MAX_ROLLUPS
  #error So many rollups... really?
#endif

#if STAGES < 0 || STAGES > MAX_STAGES
  #error So many stages... really?
#endif
#if FANS < 0 || FANS > MAX_FANS
  #error So many fans... really?
#endif
#if HEATERS < 0 || HEATERS > MAX_HEATERS
  #error So many heaters... really?
#endif

//Variables are off limits
#if (ROLLUP1_OPENING_PIN  > 7 || ROLLUP1_OPENING_PIN < 0) && ROLLUP1_OPENING_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (ROLLUP1_CLOSING_PIN  > 7 || ROLLUP1_CLOSING_PIN < 0) && ROLLUP1_CLOSING_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (ROLLUP2_OPENING_PIN  > 7 || ROLLUP2_OPENING_PIN < 0) && ROLLUP2_OPENING_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (ROLLUP2_CLOSING_PIN  > 7 || ROLLUP2_CLOSING_PIN < 0) && ROLLUP2_CLOSING_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (FAN1_PIN  > 7 || FAN1_PIN < 0) && FAN1_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (FAN2_PIN  > 7 || FAN2_PIN < 0) && FAN2_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (HEATER1_PIN  > 7 || HEATER1_PIN) < 0 && HEATER1_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (HEATER2_PIN  > 7 || HEATER2_PIN) < 0 && HEATER2_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif
#if (ALARM_PIN  > 7 || ALARM_PIN) < 0 && ALARM_PIN != OFF && defined(I2C_OUTPUTS)
  #error Check your pinout...
#endif

//Timepoints
#if (DIF_TYPE != SR && DIF_TYPE != CLOCK && DIF_TYPE != SS)
  #error Wrong timepoint type
#endif

#if (DIF_HR_MOD > 23 || DIF_HR_MOD < 0 && DIF_TYPE == CLOCK)
  #error Wrong timepoints hour value
#elif(DIF_HR_MOD > 23 || DIF_HR_MOD < -23)
  #error Wrong timepoints hour value
#endif

#if (DIF_MN_MOD > 59 || DIF_MN_MOD < 0 && DIF_TYPE == CLOCK)
  #error Wrong timepoints minut value
#elif(DIF_MN_MOD > 59 || DIF_MN_MOD < -59)
  #error Wrong timepoints minut value
#endif

#if (DIF_HEAT > 50 || DIF_HEAT < 0)
  #error Check your timepoints parameter : thats damn hot/cold
#endif
#if (DIF_COOL > 50 || DIF_HEAT < COOL)
  #error Check your timepoints parameter : thats damn hot/cold
#endif


#if (DAY_TYPE != SR && DAY_TYPE != CLOCK && DAY_TYPE != SS)
  #error Wrong timepoint type
#endif

#if (DAY_HR_MOD > 23 || DAY_HR_MOD < 0 && DAY_TYPE == CLOCK)
  #error Wrong timepoints hour value
#elif(DAY_HR_MOD > 23 || DAY_HR_MOD < -23)
  #error Wrong timepoints hour value
#endif

#if (DAY_MN_MOD > 59 || DAY_MN_MOD < 0 && DAY_TYPE == CLOCK)
  #error Wrong timepoints minut value
#elif(DAY_MN_MOD > 59 || DAY_MN_MOD < -59)
  #error Wrong timepoints minut value
#endif

#if (DAY_HEAT > 50 || DAY_HEAT < 0)
  #error Check your timepoints parameter : thats damn hot/cold
#endif
#if (DAY_COOL > 50 || DAY_HEAT < COOL)
  #error Check your timepoints parameter : thats damn hot/cold
#endif


#if (PREN_TYPE != SR && PREN_TYPE != CLOCK && PREN_TYPE != SS)
  #error Wrong timepoint type
#endif

#if (PREN_HR_MOD > 23 || PREN_HR_MOD < 0 && PREN_TYPE == CLOCK)
  #error Wrong timepoints hour value
#elif(PREN_HR_MOD > 23 || PREN_HR_MOD < -23)
  #error Wrong timepoints hour value
#endif

#if (PREN_MN_MOD > 59 || PREN_MN_MOD < 0 && PREN_TYPE == CLOCK)
  #error Wrong timepoints minut value
#elif(PREN_MN_MOD > 59 || PREN_MN_MOD < -59)
  #error Wrong timepoints minut value
#endif

#if (PREN_HEAT > 50 || PREN_HEAT < 0)
  #error Check your timepoints parameter : thats damn hot/cold
#endif
#if (PREN_COOL > 50 || PREN_HEAT < COOL)
  #error Check your timepoints parameter : thats damn hot/cold
#endif


#if (NIGHT_TYPE != SR && NIGHT_TYPE != CLOCK && NIGHT_TYPE != SS)
  #error Wrong timepoint type
#endif

#if (NIGHT_HR_MOD > 23 || NIGHT_HR_MOD < 0 && NIGHT_TYPE == CLOCK)
  #error Wrong timepoints hour value
#elif(NIGHT_HR_MOD > 23 || NIGHT_HR_MOD < -23)
  #error Wrong timepoints hour value
#endif

#if (NIGHT_MN_MOD > 59 || NIGHT_MN_MOD < 0 && NIGHT_TYPE == CLOCK)
  #error Wrong timepoints minut value
#elif(NIGHT_MN_MOD > 59 || NIGHT_MN_MOD < -59)
  #error Wrong timepoints minut value
#endif

#if (NIGHT_HEAT > 50 || NIGHT_HEAT < 0)
  #error Check your timepoints parameter : thats damn hot/cold
#endif
#if (NIGHT_COOL > 50 || NIGHT_HEAT < COOL)
  #error Check your timepoints parameter : thats damn hot/cold
#endif

#if (TP5_TYPE != SR && TP5_TYPE != CLOCK && TP5_TYPE != SS)
  #error Wrong timepoint type
#endif

#if (TP5_HR_MOD > 23 || TP5_HR_MOD < 0 && TP5_TYPE == CLOCK)
  #error Wrong timepoints hour value
#elif(TP5_HR_MOD > 23 || TP5_HR_MOD < -23)
  #error Wrong timepoints hour value
#endif

#if (TP5_MN_MOD > 59 || TP5_MN_MOD < 0 && TP5_TYPE == CLOCK)
  #error TP5_MN_MOD OFF LIMIT
#elif(TP5_MN_MOD > 59 || TP5_MN_MOD < -59)
  #error   TP5_MN_MOD OFF LIMIT

#endif

#if (TP5_HEAT > 50 || TP5_HEAT < 0)
  #error TP5_HEAT OFF LIMIT : thats damn hot/cold
#endif
#if (TP5_COOL > 50 || TP5_HEAT < COOL)
  #error TP5_COOL OFF LIMIT : thats damn hot/cold
#endif

//Rollups
#if R1_HYST > 5 || R1_HYST < 0
  #error R1_HYST OFF LIMITS
#endif
#if R1_ROTUP > 500 || R1_ROTUP < 0
  #error R1_ROTUP OFF LIMITS
#endif
#if R1_ROTDOWN > 500 || R1_ROTDOWN < 0
  #error R1_ROTDOWN OFF LIMITS
#endif
#if R1_PAUSE > 240 || R1_PAUSE < 0
  #error R1_PAUSE OFF LIMITS
#endif
#if R2_HYST > 5 || R2_HYST < 0
  #error R2_HYST OFF LIMITS
#endif
#if R2_ROTUP > 500 || R2_ROTUP < 0
  #error R2_ROTUP OFF LIMITS
#endif
#if R2_ROTDOWN > 500 || R2_ROTDOWN < 0
  #error R2_ROTDOWN OFF LIMITS
#endif
#if R2_PAUSE > 240 || R2_PAUSE < 0
  #error R2_PAUSE OFF LIMITS
#endif

//Stages
#if R1_S0_MOD >  5 ||  R1_S0_MOD < 0
  #error R1_S0_MOD OFF LIMITS
#endif
#if R1_S0_TARGET >  100 ||  R1_S0_TARGET < 0
  #error R1_S0_TARGET OFF LIMITS
#endif
#if R1_S1_MOD >  5 ||  R1_S1_MOD < 0 || R1_S1_MOD < R1_S0_MOD
  #error R1_S1_MOD OFF LIMITS
#endif
#if R1_S1_TARGET >  100 ||  R1_S1_TARGET < 0 || R1_S1_TARGET < R1_S0_TARGET
  #error R1_S1_TARGET OFF LIMITS
#endif
#if R1_S2_MOD >  5 ||  R1_S2_MOD < 0 || R1_S2_MOD < R1_S1_MOD
  #error R1_S2_MOD OFF LIMITS
#endif
#if R1_S2_TARGET >  100 ||  R1_S2_TARGET < 0 || R1_S2_TARGET < R1_S1_TARGET
  #error R1_S2_TARGET OFF LIMITS
#endif
#if R1_S3_MOD >  5 ||  R1_S3_MOD < 0 || R1_S3_MOD < R1_S2_MOD
  #error R1_S3_MOD OFF LIMITS
#endif
#if R1_S3_TARGET >  100 ||  R1_S3_TARGET < 0 || R1_S3_TARGET < R1_S2_TARGET
  #error R1_S3_TARGET OFF LIMITS
#endif
#if R1_S4_MOD >  5 ||  R1_S4_MOD < 0 || R1_S4_MOD < R1_S3_MOD
  #error R1_S4_MOD OFF LIMITS
#endif
#if R1_S4_TARGET >  100 ||  R1_S4_TARGET < 0 || R1_S4_TARGET < R1_S3_TARGET
  #error R1_S4_TARGET OFF LIMITS
#endif
#if R2_S0_MOD >  5 ||  R2_S0_MOD < 0
  #error R2_S0_MOD OFF LIMITS
#endif
#if R2_S0_TARGET >  100 ||  R2_S0_TARGET < 0
  #error R2_S0_TARGET OFF LIMITS
#endif
#if R2_S1_MOD >  5 ||  R2_S1_MOD < 0 || R2_S1_MOD < R2_S0_MOD
  #error R2_S1_MOD OFF LIMITS
#endif
#if R2_S1_TARGET >  100 ||  R2_S1_TARGET < 0 || R2_S1_TARGET < R2_S0_TARGET
  #error R2_S1_TARGET OFF LIMITS
#endif
#if R2_S2_MOD >  5 ||  R2_S2_MOD < 0 || R2_S2_MOD < R2_S1_MOD
  #error R2_S2_MOD OFF LIMITS
#endif
#if R2_S2_TARGET >  100 ||  R2_S2_TARGET < 0 || R2_S2_TARGET < R2_S1_TARGET
  #error R2_S2_TARGET OFF LIMITS
#endif
#if R2_S3_MOD >  5 ||  R2_S3_MOD < 0 || R2_S3_MOD < R2_S2_MOD
  #error R2_S3_MOD OFF LIMITS
#endif
#if R2_S3_TARGET >  100 ||  R2_S3_TARGET < 0 || R2_S3_TARGET < R2_S2_TARGET
  #error R2_S3_TARGET OFF LIMITS
#endif
#if R2_S4_MOD >  5 ||  R2_S4_MOD < 0 || R2_S4_MOD < R2_S3_MOD
  #error R2_S4_MOD OFF LIMITS
#endif
#if R2_S4_TARGET >  100 ||  R2_S4_TARGET < 0 || R2_S4_TARGET < R2_S3_TARGET
  #error R2_S4_TARGET OFF LIMITS
#endif

//Fans
#if F1_HYST > 5 || F1_HYST < 0
  #error F1_HYST OFF LIMITS
#endif
#if F1_MOD > 10 || F1_MOD < -5
  #error F1_MOD OFF LIMITS
#endif
#if F2_HYST > 5 || F2_HYST < 0
  #error F2_HYST OFF LIMITS
#endif
#if F2_MOD > 10 || F2_MOD < -5
  #error F2_MOD OFF LIMITS
#endif

//Heaters
#if H1_HYST > 5 || H1_HYST < 0
  #error H1_HYST OFF LIMITS
#endif
#if H1_MOD > 5 || H1_MOD < -10
  #error H1_MOD OFF LIMITS
#endif
#if H2_HYST > 5 || H2_HYST < 0
  #error H2_HYST OFF LIMITS
#endif
#if H2_MOD > 5 || H2_MOD < -10
  #error H2_MOD OFF LIMITS
#endif
