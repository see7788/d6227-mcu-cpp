#ifndef a7129namespace_h
#define a7129namespace_h
#include <Arduino.h>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <freertos/queue.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <myStruct_t.h>
#define SYSTEMCLOCK_REG 0x00
#define PLL1_REG 0x01
#define PLL2_REG 0x02
#define PLL3_REG 0x03
#define PLL4_REG 0x04
#define PLL5_REG 0x05
#define PLL6_REG 0x06
#define CRYSTAL_REG 0x07
#define PAGEA_REG 0x08
#define PAGEB_REG 0x09
#define RX1_REG 0x0A
#define RX2_REG 0x0B
#define ADC_REG 0x0C
#define PIN_REG 0x0D
#define CALIBRATION_REG 0x0E
#define MODE_REG 0x0F

#define TX1_PAGEA 0x00
#define WOR1_PAGEA 0x01
#define WOR2_PAGEA 0x02
#define RFI_PAGEA 0x03
#define PM_PAGEA 0x04
#define RTH_PAGEA 0x05
#define AGC1_PAGEA 0x06
#define AGC2_PAGEA 0x07
#define GIO_PAGEA 0x08
#define CKO_PAGEA 0x09
#define VCB_PAGEA 0x0A
#define CHG1_PAGEA 0x0B
#define CHG2_PAGEA 0x0C
#define FIFO_PAGEA 0x0D
#define CODE_PAGEA 0x0E
#define WCAL_PAGEA 0x0F

#define TX2_PAGEB 0x00
#define IF1_PAGEB 0x01
#define IF2_PAGEB 0x02
#define ACK_PAGEB 0x03
#define ART_PAGEB 0x04

#define CMD_Reg_W 0x00  // 000x,xxxx control register write
#define CMD_Reg_R 0x80  // 100x,xxxx control register read
#define CMD_ID_W 0x20   // 001x,xxxx ID write
#define CMD_ID_R 0xA0   // 101x,xxxx ID Read
#define CMD_FIFO_W 0x40 // 010x,xxxx TX FIFO Write
#define CMD_FIFO_R 0xC0 // 110x,xxxx RX FIFO Read
#define CMD_RF_RST 0x70 // x111,xxxx RF reset
#define CMD_TFR 0x60    // 0110,xxxx TX FIFO address pointrt reset
#define CMD_RFR 0xE0    // 1110,xxxx RX FIFO address pointer reset

#define CMD_SLEEP 0x10 // 0001,0000 SLEEP mode
#define CMD_IDLE 0x12  // 0001,0010 IDLE mode
#define CMD_STBY 0x14  // 0001,0100 Standby mode
#define CMD_PLL 0x16   // 0001,0110 PLL mode
#define CMD_RX 0x18    // 0001,1000 RX mode
#define CMD_TX 0x1A    // 0001,1010 TX mode
// #define CMD_DEEP_SLEEP  0x1C  //0001,1100 Deep Sleep mode(tri-state)
#define CMD_DEEP_SLEEP 0x1F // 0001,1111 Deep Sleep mode(pull-high)
/////////////////////////////////////////////////////////////////////////////////////////////寄存器地址
#define Uint8 unsigned char
#define Uint16 unsigned int
// #define Sint32 signed long
// #define Uint32 unsigned long

/////////////////////////////////////////////////////////////////////////////////////类型转换等

#define TIMEOUT 50  // 50ms
#define t0hrel 1000 // 1ms
#define SDIO 32
#define SCS 14
#define SCK 33
#define CKO 36
#define GIO1 34
#define GIO2 39

namespace a7129namespace
{
    /*********************************************************************
    **  Global Variable Declaration
    *********************************************************************/

    Uint8 A7129_RX_BUFF[64];
    SemaphoreHandle_t a7129_rx_semaphore;
    typedef uint32_t id_t;
    typedef Uint8 type_t;
    typedef Uint8 state_t;
    typedef struct
    {
        id_t id;
        type_t type;
        state_t state;
    } idInfo_t;
    typedef std::unordered_map<id_t, idInfo_t> idsInfo_t;
    idsInfo_t idsInfo;
    // sendTo_name,id白名单
    typedef std::tuple<String, idsInfo_t> config_t;
    typedef struct
    {
        config_t &config;
        //QueueHandle_t &myStructQueueHandle;
        std::function<void(void)> startCallback;
        std::function<void(void)> tickCallBack;
    } rootTaskParam_t;
    const Uint16 A7129Config[] = // 433MHz, 100kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
        {
            0x0021, // SYSTEM CLOCK register,
            0x0A21, // PLL1 register,
            0xDA05, // PLL2 register,  433.301MHz
            0x0000, // PLL3 register,
            0x0A20, // PLL4 register,
            0x0024, // PLL5 register,
            0x0000, // PLL6 register,
            0x0001, // CRYSTAL register,
            0x0000, // PAGEA,
            0x0000, // PAGEB,
            0x18D4, // RX1 register,   IFBW=100KHz, ETH=1
            0x7009, // RX2 register,   by preamble
            0x4400, // ADC register,
            0x0800, // PIN CONTROL register,   Use Strobe CMD
            0x4845, // CALIBRATION register,
            0x20C0  // MODE CONTROL register,  Use FIFO mode
    };

    const Uint16 A7129Config_PageA[] = // 433MHz, 100kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
        {
            0x1706, // TX1 register,   Fdev = 37.5kHz
            0x0000, // WOR1 register,
            0x0000, // WOR2 register,
            0x1187, // RFI register,   Enable Tx Ramp up/down
            0x0170, // PM register,
            0x0302, // RTH register,
            0x400F, // AGC1 register,
            0x0AC0, // AGC2 register,
            0x0045, // GIO register,
            0xD281, // CKO register
            0x0004, // VCB register,
            0x0A21, // CHG1 register,  430MHz
            0x0022, // CHG2 register,  435MHz
            0x003F, // FIFO register,  FEP=63+1=64bytes
            0x1507, // CODE register,  Preamble=4bytes, ID=4bytes
            0x0000  // WCAL register,
    };

    const Uint16 A7129Config_PageB[] = // 433MHz, 100kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
        {
            0x0B37, // TX2 register,
            0x8400, // IF1 register,   Enable Auto-IF, IF=200KHz
            0x0000, // IF2 register,
            0x0000, // ACK register,
            0x0000  // ART register,
    };

    /*********************************************************************
    **  function Declaration
    *********************************************************************/
    void StrobeCMD(Uint8 cmd)
    {
        Uint8 i;

        pinMode(SDIO, OUTPUT); // SDIO写输出
        digitalWrite(SCS, LOW);

        for (i = 0; i < 8; i++)
        {
            if (cmd & 0x80)
                digitalWrite(SDIO, HIGH);
            else
                digitalWrite(SDIO, LOW);

            NOP();

            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);

            cmd <<= 1;
        }
        digitalWrite(SCS, HIGH);
    }

    void ByteSend(Uint8 src)
    {
        Uint8 i;

        pinMode(SDIO, OUTPUT); // SDIO写输出
        for (i = 0; i < 8; i++)
        {
            if (src & 0x80)
                digitalWrite(SDIO, HIGH);
            else
                digitalWrite(SDIO, LOW);

            NOP();

            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);

            src <<= 1;
        }
    }

    Uint8 ByteRead(void)
    {
        Uint8 i, tmp;

        // read data code

        digitalWrite(SDIO, HIGH); // SDIO拉高
        pinMode(SDIO, INPUT);     // SDIO写输入
        for (i = 0; i < 8; i++)
        {
            if (digitalRead(SDIO))
                tmp = (tmp << 1) | 0x01;
            else
                tmp = tmp << 1;
            NOP();
            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);
        }
        return tmp;
    }

    void A7129_WriteReg(Uint8 address, Uint16 dataWord)
    {
        Uint8 i;
        pinMode(SDIO, OUTPUT); // SDIO写 输出
        digitalWrite(SCS, LOW);

        address |= CMD_Reg_W;
        for (i = 0; i < 8; i++)
        {
            if (address & 0x80)
                digitalWrite(SDIO, HIGH);
            else
                digitalWrite(SDIO, LOW);
            NOP();
            digitalWrite(SCK, HIGH);

            NOP();
            digitalWrite(SCK, LOW);

            address <<= 1;
        }
        NOP();

        // send data word
        for (i = 0; i < 16; i++)
        {
            if (dataWord & 0x8000)
                digitalWrite(SDIO, HIGH);
            else
                digitalWrite(SDIO, LOW);
            NOP();
            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);

            dataWord <<= 1;
        }
        digitalWrite(SCS, HIGH);
    }

    Uint16 A7129_ReadReg(Uint8 address)
    {
        Uint8 i;
        Uint16 tmp;

        pinMode(SDIO, OUTPUT); // SDIO写输出
        digitalWrite(SCS, LOW);

        address |= CMD_Reg_R;
        for (i = 0; i < 8; i++)
        {
            if (address & 0x80)
                digitalWrite(SDIO, HIGH);
            else
                digitalWrite(SDIO, LOW);

            NOP();
            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);

            address <<= 1;
        }
        // read data code
        digitalWrite(SDIO, HIGH); // SDIO拉高
        pinMode(SDIO, INPUT);     // SDIO写输入
        NOP();
        for (i = 0; i < 16; i++)
        {
            if (digitalRead(SDIO))
                tmp = (tmp << 1) | 0x01;
            else
                tmp = tmp << 1;
            NOP();
            digitalWrite(SCK, HIGH);
            NOP();

            digitalWrite(SCK, LOW);
        }
        digitalWrite(SCS, HIGH);

        return tmp;
    }

    void A7129_WritePageA(Uint8 address, Uint16 dataWord)
    {
        Uint16 tmp;

        tmp = address;
        tmp = ((tmp << 12) | A7129Config[CRYSTAL_REG]);
        A7129_WriteReg(CRYSTAL_REG, tmp);
        A7129_WriteReg(PAGEA_REG, dataWord);
    }

    Uint16 A7129_ReadPageA(Uint8 address)
    {
        Uint16 tmp;

        tmp = address;
        tmp = ((tmp << 12) | A7129Config[CRYSTAL_REG]);
        A7129_WriteReg(CRYSTAL_REG, tmp);
        tmp = A7129_ReadReg(PAGEA_REG);
        return tmp;
    }

    void A7129_WritePageB(Uint8 address, Uint16 dataWord)
    {
        Uint16 tmp;

        tmp = address;
        tmp = ((tmp << 7) | A7129Config[CRYSTAL_REG]);
        A7129_WriteReg(CRYSTAL_REG, tmp);
        A7129_WriteReg(PAGEB_REG, dataWord);
    }

    Uint16 A7129_ReadPageB(Uint8 address)
    {
        Uint16 tmp;

        tmp = address;
        tmp = ((tmp << 7) | A7129Config[CRYSTAL_REG]);
        A7129_WriteReg(CRYSTAL_REG, tmp);
        tmp = A7129_ReadReg(PAGEB_REG);
        return tmp;
    }

    void A7129_Config(void)
    {
        Uint8 i;
        Uint16 tmp;

        for (i = 0; i < 8; i++)
            A7129_WriteReg(i, A7129Config[i]);

        for (i = 10; i < 16; i++)
            A7129_WriteReg(i, A7129Config[i]);

        for (i = 0; i < 16; i++)
            A7129_WritePageA(i, A7129Config_PageA[i]);

        for (i = 0; i < 5; i++)
            A7129_WritePageB(i, A7129Config_PageB[i]);

        // for check
        tmp = A7129_ReadReg(SYSTEMCLOCK_REG);
        if (tmp != A7129Config[SYSTEMCLOCK_REG])
        {
            //  ESP_LOGE("DEBUE", "%s", "");
        }
    }

    void A7129_WriteID(void)
    {
        Uint8 i;
        Uint8 d1, d2, d3, d4;
        Uint8 ID_Tab[8] = {0x54, 0x21, 0xA4, 0x23, 0xC7, 0x33, 0x45, 0xE7}; // ID code
        digitalWrite(SCS, LOW);

        ByteSend(CMD_ID_W);
        for (i = 0; i < 4; i++)
            ByteSend(ID_Tab[i]);
        digitalWrite(SCS, HIGH);

        NOP();
        digitalWrite(SCS, LOW);

        ByteSend(CMD_ID_R);
        d1 = ByteRead();
        d2 = ByteRead();
        d3 = ByteRead();
        d4 = ByteRead();
        digitalWrite(SCS, HIGH);

        if ((d1 != ID_Tab[0]) || (d2 != ID_Tab[1]) || (d3 != ID_Tab[2]) || (d4 != ID_Tab[3]))
        {
            ESP_LOGE("DEBUE", "%s", "");
        }
    }

    void A7129_Cal(void)
    {
        Uint8 fb, fcd, fbcf; // IF Filter
        Uint8 vb, vbcf;      // VCO Current
        Uint8 vcb, vccf;     // VCO Band
        Uint16 tmp;

        // IF calibration procedure @STB state
        A7129_WriteReg(MODE_REG, A7129Config[MODE_REG] | 0x0802); // IF Filter & VCO Current Calibration
        tmp = A7129_ReadReg(MODE_REG);
        // TEST_BUFF[2] = tmp;    //测试

        do
        {
            tmp = A7129_ReadReg(MODE_REG); // 等待时读0x00C2/0x08C2,直到0x00C0
        } while (tmp & 0x0002);            // 初始化重点，MODE_REG寄存器的第2bit位是等待VCO组校准自动停止的，校准后才可以走下面的配置

        //    TEST_BUFF[1] = tmp;   //测试
        // for check(IF Filter)
        tmp = A7129_ReadReg(CALIBRATION_REG); // 读到0x1186
        //    TEST_BUFF[0] = tmp;   //测试
        fb = tmp & 0x0F;
        fcd = (tmp >> 11) & 0x1F;
        fbcf = (tmp >> 4) & 0x01;
        if (fbcf)
        {
            ESP_LOGE("DEBUE", "fbcf", "");
        }

        // for check(VCO Current)
        tmp = A7129_ReadPageA(VCB_PAGEA); // 读到0x0002
        //    TEST_BUFF[0] = tmp;       //测试
        vcb = tmp & 0x0F;
        vccf = (tmp >> 4) & 0x01;
        //    TEST_BUFF[4] = vccf;        //测试
        if (vccf)
        {
            ESP_LOGE("DEBUE", "vccf", "");
        }
        // RSSI Calibration procedure @STB state
        A7129_WriteReg(ADC_REG, 0x4C00); // set ADC average=64
        // 样品没有这两句段
        //  A7129_WritePageA(WOR2_PAGEA, 0xF800);               //set RSSC_D=40us and RS_DLY=80us
        //  A7129_WritePageA(TX1_PAGEA, A7129Config_PageA[TX1_PAGEA] | 0xE000); //set RC_DLY=1.5ms
        A7129_WriteReg(MODE_REG, A7129Config[MODE_REG] | 0x1000); // RSSI Calibration
        do
        {
            tmp = A7129_ReadReg(MODE_REG);
        } while (tmp & 0x1000);
        A7129_WriteReg(ADC_REG, A7129Config[ADC_REG]);
        // 样品没有这两句段
        //  A7129_WritePageA(WOR2_PAGEA, A7129Config_PageA[WOR2_PAGEA]);
        //  A7129_WritePageA(TX1_PAGEA, A7129Config_PageA[TX1_PAGEA]);

        // VCO calibration procedure @STB state
        A7129_WriteReg(PLL1_REG, A7129Config[PLL1_REG]);
        A7129_WriteReg(PLL2_REG, A7129Config[PLL2_REG]);
        A7129_WriteReg(MODE_REG, A7129Config[MODE_REG] | 0x0004); // VCO Band Calibration
        do
        {
            tmp = A7129_ReadReg(MODE_REG);
        } while (tmp & 0x0004);

        // for check(VCO Band)
        tmp = A7129_ReadReg(CALIBRATION_REG); // 读到0x1286
        vb = (tmp >> 5) & 0x07;
        vbcf = (tmp >> 8) & 0x01;
        if (vbcf)
        {
            ESP_LOGE("DEBUE", "vbcf", "");
        }
    }

    void A7129_WriteFIFO(Uint8 db[8])
    {
        Uint8 i;

        StrobeCMD(CMD_TFR); // TX FIFO address pointer reset

        digitalWrite(SCS, LOW);

        ByteSend(CMD_FIFO_W); // TX FIFO write command
        for (i = 0; i < 7; i++)
            ByteSend(db[i]);
        digitalWrite(SCS, HIGH);
    }

    void RxPacket(void)
    {
        Uint8 i;
        StrobeCMD(CMD_RFR); // RX FIFO address pointer reset
        digitalWrite(SCS, LOW);

        ByteSend(CMD_FIFO_R);                       // RX FIFO read command
        for (i = 0; i < sizeof(A7129_RX_BUFF); i++) // 只接收7个数据即可
        {
            A7129_RX_BUFF[i] = ByteRead();
        }
        digitalWrite(SCS, HIGH);
    }

    void InitRF(void)
    {

        // initial pin
        pinMode(SCS, OUTPUT);
        digitalWrite(SCS, HIGH);

        pinMode(SCK, OUTPUT);
        digitalWrite(SCK, HIGH);
        // SDIO写输出
        pinMode(SDIO, OUTPUT);
        digitalWrite(SDIO, HIGH);
        pinMode(GIO1, INPUT_PULLUP);

        StrobeCMD(CMD_RF_RST); // reset A7129 chip
        A7129_Config();        // config A7129 chip
        delay(1);              // for crystal stabilized
        A7129_WriteID();       // write ID code
        delayMicroseconds(1);  // for crystal stabilized
        A7129_Cal();           // IF and VCO calibration
        delayMicroseconds(1);  // for crystal stabilized
    }

    /********/
    Uint8 CRC16_High, CRC16_Low;
    Uint8 CRC16_LookupHigh[16] = {
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
        0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1};
    Uint8 CRC16_LookupLow[16] = {
        0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
        0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF};
    void CRC16_Init(void)
    {
        CRC16_High = 0x1D;
        CRC16_Low = 0x0F;
    }
    void CRC16_Update4Bits(Uint8 val)
    {
        Uint8 t;
        // Step one, extract the Most significant 4 bits of the CRC register
        t = CRC16_High >> 4;
        // XOR in the Message Data into the extracted bits
        t = t ^ val;
        // Shift the CRC Register left 4 bits
        CRC16_High = (CRC16_High << 4) | (CRC16_Low >> 4);
        CRC16_Low = CRC16_Low << 4;
        // Do the table lookups and XOR the result into the CRC Tables
        CRC16_High = CRC16_High ^ CRC16_LookupHigh[t];
        CRC16_Low = CRC16_Low ^ CRC16_LookupLow[t];
    }

    void CRC16_Update(Uint8 val)
    {
        CRC16_Update4Bits(val >> 4);   // High nibble first
        CRC16_Update4Bits(val & 0x0F); // Low nibble
    }

    void CRC_test(Uint8 *Payload, Uint8 length)
    {
        Uint8 i;
        CRC16_Init();
        for (i = 0; i < length; i++)  // length--> the payload length
            CRC16_Update(Payload[i]); // Payload--> the data you send
    }

    int yblCrc() // 数据输出
    {
        CRC_test(A7129_RX_BUFF, 3);
        // 深圳市易百珑科技有限公司
        if ((CRC16_High == A7129_RX_BUFF[3]) && (CRC16_Low == A7129_RX_BUFF[4]))
        {
            CRC_test(A7129_RX_BUFF, 6);
            if (CRC16_Low == A7129_RX_BUFF[6])
            {
                id_t id_val = A7129_RX_BUFF[0] << 24 | A7129_RX_BUFF[1] << 16 | (A7129_RX_BUFF[5] & 0x0f) << 8 | ((A7129_RX_BUFF[5] & 0x3f) >> 4) * 2;
                if (idsInfo.empty() || idsInfo.count(id_val) > 0)
                {
                state_t state_val = A7129_RX_BUFF[2];
                type_t type_val = (A7129_RX_BUFF[5] >> 6) + 1;
                idsInfo[id_val] = {
                    .id = id_val,
                    .type = type_val,
                    .state = state_val};
                }
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xSemaphoreGiveFromISR(a7129_rx_semaphore, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(a7129_rx_semaphore);
                return (0); // 接收正确
            }
            else
            {
                return (1); // 接收错误
            }
        }
        else
        {
            return (2); // 接收错误
        }
    }

    void yblSend(id_t id)
    {
        idInfo_t idInfo;
        StrobeCMD(CMD_TX);
        Uint8 db[8]; // 需要发送的数据
        // id1  id2  state  else  else type
        db[0] = idInfo.id >> 24;
        db[1] = (idInfo.id >> 16) & 0xff;
        db[2] = idInfo.state;
        db[3] = (idInfo.id >> 8) & 0xff;
        db[4] = idInfo.id & 0xff;
        db[5] = idInfo.type;
        db[6] = 0xff;
        InitRF();
        A7129_WriteFIFO(db); // write data to TX FIFO
        StrobeCMD(CMD_STBY);
        StrobeCMD(CMD_PLL);
        StrobeCMD(CMD_RX); // 设置为接收模式
    }

    void yblInterrupt(void)
    {
        RxPacket(); // 接收数据,数据存放在A7129_RX_BUFF[],数组最多7个元素
        StrobeCMD(CMD_RX);
        yblCrc();
    }
    void rootTask(void *ptr)
    {
        rootTaskParam_t *rootTaskParam = (rootTaskParam_t *)ptr;
        InitRF(); // init RF,最后一个字段0x8E,0x12,0x86
        StrobeCMD(CMD_STBY);
        StrobeCMD(CMD_PLL);
        pinMode(GIO1, INPUT_PULLUP);
        a7129_rx_semaphore = xSemaphoreCreateBinary();
        attachInterrupt(GIO1, yblInterrupt, FALLING); // 创建中断
        StrobeCMD(CMD_RX);                            // 设为接收模式
        TickType_t lastSendTime = 0;
        int pd_tick = pdMS_TO_TICKS(800);
        rootTaskParam->startCallback();
        while (1)
        {
            if (xSemaphoreTake(a7129_rx_semaphore, portMAX_DELAY) == pdTRUE)
            {
                TickType_t currentTime = xTaskGetTickCount();
                if ((currentTime - lastSendTime) >= pd_tick)
                {
                    lastSendTime = currentTime;
                    for (const auto &pair : idsInfo)
                    {
                        ESP_LOGV("TAG", "ID: %u, Type: %u, State: %u", pair.second.id, pair.second.type, pair.second.state);
                    }
                    // myStruct_t obj = {
                    //     .sendTo_name = std::get<0>(rootTaskParam->config),
                    //     .str = "[\"ybl.State\"]"};
                    // if (xQueueSend(rootTaskParam->myStructQueueHandle, &obj, 50) != pdPASS)
                    // {
                    //     ESP_LOGV("yblInterrupt", "Queue is full");
                    // }
                    rootTaskParam->tickCallBack();
                }
            }
        }
    }
};
#endif