//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "tcp_server.h"

#include "socket.h"
#include "wizchip_conf.h"

#if TCP_SERVER_CFG_DEBUG == 1
#include "stdio.h"
#endif

//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t tcpServerRun(uint8_t sn, uint16_t port, tcpServerHandle_t handle){

    int8_t ret;
#if TCP_SERVER_CFG_DEBUG == 1
   uint8_t destip[4];
   uint16_t destport;
#endif

   switch(getSn_SR(sn))
   {
      case SOCK_ESTABLISHED :
         if(getSn_IR(sn) & Sn_IR_CON)
         {
#if TCP_SERVER_CFG_DEBUG == 1
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);

            printf("%s (sn %d): Connected - %d.%d.%d.%d : %d\r\n", __func__, sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
            setSn_IR(sn,Sn_IR_CON);
         }
         if((getSn_RX_RSR(sn)) > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
         {
            int32_t sn32 = sn;
            ret = handle(sn32);
            if(ret != 0){
                close(sn);
                return ret;
            }
         }
         break;
      case SOCK_CLOSE_WAIT :
#if TCP_SERVER_CFG_DEBUG == 1
         printf("%s (sn %d): CloseWait\r\n", __func__, sn);
#endif
         if((ret = disconnect(sn)) != SOCK_OK) return ret;
#if TCP_SERVER_CFG_DEBUG == 1
         printf("%s (sn %d): Socket closed\r\n", __func__, sn);
#endif
         break;
      case SOCK_INIT :
#if TCP_SERVER_CFG_DEBUG == 1
         printf("%s (sn %d): Listen, TCP server loopback, port [%d]\r\n", __func__, sn, port);
#endif
         if( (ret = listen(sn)) != SOCK_OK) return ret;
         break;
      case SOCK_CLOSED:
#if TCP_SERVER_CFG_DEBUG == 1
         printf("%s (sn %d): TCP server loopback start\r\n", __func__, sn);
#endif
         if((ret = socket(sn, Sn_MR_TCP, port, SF_TCP_NODELAY)) != sn) return ret;
#if TCP_SERVER_CFG_DEBUG == 1
         printf("%s (sn %d): Socket opened\r\n", __func__, sn);
#endif
         break;
      default:
         break;
   }
   
   return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================

