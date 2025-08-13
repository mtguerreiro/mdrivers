
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "ledws2812.h"

#include "pwm_irq_handler.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#include "hardware/pwm.h"
//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#define LED_WS2812_CONFIG_DBG       0
#define LED_WS2812_CONFIG_DBG_PIN   12

#if LED_WS2812_CONFIG_DBG == 1
#include "hardware/gpio.h"
#endif

#define LED_WS2812_CONFIG_PWM_SLICE             1
#define LED_WS2812_CONFIG_PWM_CLK_DIV           125.0f
#define LED_WS2812_CONFIG_PWM_TICK_PERIOD_US    ( (float)(LED_WS2812_CONFIG_PWM_CLK_DIV / 125.f) )

#define LED_WS2812_CONFIG_MAX_INTENSITY         4.0f

#define LED_WS2812_CONFIG_LED_UPDATE_PER        (10000.0f / LED_WS2812_CONFIG_PWM_TICK_PERIOD_US)

#define LED_WS2812_CONFIG_IS_RGBW               false
#define LED_WS2812_CONFIG_PIN            2

typedef enum{
    LED_WS2812_SM_STATE_SET,
    LED_WS2812_SM_STATE_RESET,
}ledws2812Sm_t;

typedef struct{

    uint32_t nActiveLeds;

    uint16_t ton;
    uint16_t toff;
    
    uint8_t rgb[3];
}ledws2812Control_t;
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static ledws2812Control_t gxlc = {.nActiveLeds = 5, .rgb = {0}};

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void ledws2812InitializeTimer(void);

static void ledws2812TimerSet(uint16_t val);
static void ledws2812TimerSetEnable(bool enable);

static int32_t ledws2812SetNumberActiveLeds(uint32_t nleds);

static void ledws2812UpdateColorPwmIrq(void);

static int32_t ledws2812IfSetNumberActiveLeds(void *in, uint32_t insize, void **out, uint32_t maxoutsize);

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t ledws2812Initialize(void){

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, LED_WS2812_CONFIG_PIN, 800000, LED_WS2812_CONFIG_IS_RGBW);

    ledws2812InitializeTimer();

    return 0;
}
//-----------------------------------------------------------------------------
uint32_t ledws2812GetNumberLeds(void){

    return 1;
}
//-----------------------------------------------------------------------------
int32_t ledws2812SetIntensity(uint8_t led, uint8_t intensity, uint32_t to){
  
    if( intensity > LED_WS2812_CONFIG_MAX_INTENSITY ) intensity = LED_WS2812_CONFIG_MAX_INTENSITY;

    gxlc.ton = (uint16_t) ( ((float)intensity) / LED_WS2812_CONFIG_MAX_INTENSITY * LED_WS2812_CONFIG_LED_UPDATE_PER);
    gxlc.toff = (uint16_t) ( (LED_WS2812_CONFIG_MAX_INTENSITY - ((float)intensity)) / LED_WS2812_CONFIG_MAX_INTENSITY * LED_WS2812_CONFIG_LED_UPDATE_PER);

    return 0;
}
//-----------------------------------------------------------------------------
int32_t ledws2812SetColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue, uint32_t to){

    gxlc.rgb[0] = red;
    gxlc.rgb[1] = green;
    gxlc.rgb[2] = blue;

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void ledws2812InitializeTimer(void){

#if LED_WS2812_CONFIG_DBG == 1
    gpio_init(LED_WS2812_CONFIG_DBG_PIN);
    gpio_set_dir(LED_WS2812_CONFIG_DBG_PIN, GPIO_OUT);
    gpio_put(LED_WS2812_CONFIG_DBG_PIN, 0);
#endif

    pwmIrqHandlerSliceEnable(LED_WS2812_CONFIG_PWM_SLICE);
    pwmIrqHandlerAdd(LED_WS2812_CONFIG_PWM_SLICE, ledws2812UpdateColorPwmIrq);
    
    pwm_config config = pwm_get_default_config();

    /* Set divider and initial clock div */
    pwm_config_set_clkdiv(&config, (float)LED_WS2812_CONFIG_PWM_CLK_DIV);
    pwm_config_set_wrap(&config, 0xFFF);

    /* Load configurations */
    pwm_init(LED_WS2812_CONFIG_PWM_SLICE, &config, false);

    ledws2812TimerSet(LED_WS2812_CONFIG_LED_UPDATE_PER);
    ledws2812TimerSetEnable(true);
}
//-----------------------------------------------------------------------------
static void ledws2812TimerSet(uint16_t val){

    pwm_set_wrap(LED_WS2812_CONFIG_PWM_SLICE, val - 1);
}
//-----------------------------------------------------------------------------
static void ledws2812TimerSetEnable(bool enable){

    pwm_set_counter(LED_WS2812_CONFIG_PWM_SLICE, 0);
    pwm_set_enabled(LED_WS2812_CONFIG_PWM_SLICE, enable);
}
//-----------------------------------------------------------------------------
static int32_t ledws2812SetNumberActiveLeds(uint32_t nleds){

    if( nleds > LED_WS2812_CFG_MAX_ACTIVE_LEDS ) return -1;

    gxlc.nActiveLeds = nleds;

    return 0;
}
//-----------------------------------------------------------------------------
static int32_t ledws2812IfSetNumberActiveLeds(void *in, uint32_t insize, void **out, uint32_t maxoutsize){

    (void)maxoutsize;
    int32_t status;

    uint32_t nleds = *( (uint32_t *)in );

    uint32_t *o = (uint32_t *)*out;

    status = ledws2812SetNumberActiveLeds(nleds);

    *o = status;

    return 4;
}
//-----------------------------------------------------------------------------
static void ledws2812UpdateColorPwmIrq(void){

    uint32_t k;
    static uint32_t activeLeds = 1;
    static int state = LED_WS2812_SM_STATE_SET;
    uint16_t pwmPeriod = LED_WS2812_CONFIG_LED_UPDATE_PER;

#if LED_WS2812_CONFIG_DBG == 1
    gpio_put(LED_WS2812_CONFIG_DBG_PIN, 1);
#endif

    ledws2812TimerSetEnable(false);

    k = gxlc.nActiveLeds;

    if( state == LED_WS2812_SM_STATE_SET ){
        if( gxlc.ton == 0 ){
            pwmPeriod = LED_WS2812_CONFIG_LED_UPDATE_PER;
        }
        if( gxlc.ton != 0 ){
            while(k--) put_pixel(urgb_u32(gxlc.rgb[0], gxlc.rgb[1], gxlc.rgb[2]));
            pwmPeriod = gxlc.ton;
        }
        state = LED_WS2812_SM_STATE_RESET;
    }

    else if( state == LED_WS2812_SM_STATE_RESET ){
        if(gxlc.toff == 0 ){
            pwmPeriod = LED_WS2812_CONFIG_LED_UPDATE_PER;
        }
        else{
            while(k--) put_pixel(urgb_u32(0, 0, 0));
            pwmPeriod = gxlc.toff;
        }
        state = LED_WS2812_SM_STATE_SET;
    }

    if( gxlc.nActiveLeds != activeLeds){
        activeLeds = gxlc.nActiveLeds;
        k = LED_WS2812_CFG_MAX_ACTIVE_LEDS - gxlc.nActiveLeds;
        while(k--) put_pixel(urgb_u32(0, 0, 0));
    }

    ledws2812TimerSet(pwmPeriod);
    ledws2812TimerSetEnable(true);
    
#if LED_WS2812_CONFIG_DBG == 1
    gpio_put(LED_WS2812_CONFIG_DBG_PIN, 0);
#endif
}
//-----------------------------------------------------------------------------
//=============================================================================
