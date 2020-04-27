#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <list>
#include <unistd.h>
#include <time.h>
#include "hwsdk.h"
#include "ivs_error.h"
#include "IVS_SDK.h"
#ifndef CHECK_POINTER_VOID
  #define CHECK_POINTER_VOID(p) assert(p!=NULL);
#endif

int main(){
  
   printf("haha...\n");
   IVS_INT32 iRet = IVS_SDK_Init(); 
   printf("init=%i\n",iRet);
   IVS_INT32 iVersion=IVS_SDK_GetVersion();
   printf("version=%x\n",iVersion);
   iRet = IVS_SDK_SetLogPath("/log/sdk1");
   if (IVS_SUCCEED == iRet){
      printf("setLogPath succeed!\n");
   }
   IVS_LOGIN_INFO loginInfo = {0};
   loginInfo.stIP.uiIPType=IP_V4;
   strncpy(loginInfo.stIP.cIP,"60.191.11.249",IVS_IP_LEN);
   loginInfo.uiPort=9900;
   strncpy(loginInfo.cUserName,"tianhe",IVS_NAME_LEN);
   strncpy(loginInfo.pPWD,"Huawei@123",IVS_PWD_LEN);
   IVS_INT32 iSessionID = -1;
   iRet = IVS_SDK_Login(&loginInfo,&iSessionID);
   if (IVS_SUCCEED == iRet){
   	printf("login succeed,session id=%d\n",iSessionID);
        //srand(time(0)); 
        //sleep(rand()%30);  
      IVS_UINT32 uiReqNum = 10;//欲查询摄像机设备的个数 
        // 计算缓存大小，因为IVS_CAMERA_BRIEF_INFO_LIST中已包含一个IVS_CAMERA_BRIEF_INFO，所以乘数是(uiReqNum-1) 
      IVS_UINT32 uiRsqBufSize = sizeof(IVS_CAMERA_BRIEF_INFO_LIST) + (uiReqNum-1) * sizeof(IVS_CAMERA_BRIEF_INFO);
      IVS_CAMERA_BRIEF_INFO_LIST* pCameraInfoList = (IVS_CAMERA_BRIEF_INFO_LIST*)(new IVS_CHAR[uiRsqBufSize]);//分配内存 
      CHECK_POINTER_VOID(pCameraInfoList)//指针判空 
      memset((void*)pCameraInfoList,0,uiRsqBufSize); 
      IVS_INDEX_RANGE IndexRange = {1, uiReqNum};//查询第1到第100个 
      std::list<IVS_CAMERA_BRIEF_INFO> DeviceList; 
        //获取摄像机设备列表  
     IVS_UINT32 uiNum=-1;
     IVS_INT32 iRet = IVS_SDK_GetDeviceList(iSessionID, DEVICE_TYPE_CAMERA, &IndexRange, pCameraInfoList, uiRsqBufSize);
     if (IVS_SUCCEED == iRet) 
       { 
           //获取成功，先计算实际返回的摄像机个数
         uiNum = (pCameraInfoList->stIndexRange.uiToIndex - pCameraInfoList->stIndexRange.uiFromIndex) + 1; 
         uiNum = (uiNum > pCameraInfoList->uiTotal) ? pCameraInfoList->uiTotal : uiNum; 
         DeviceList.clear(); 
         for (IVS_UINT32 i=0; i<uiNum; i++) 
           { 
             DeviceList.push_back(pCameraInfoList->stCameraBriefInfo[i]);//保存摄像机列表 
             printf("%i: %s\t%st\%s\t %s,%s\n",i,pCameraInfoList->stCameraBriefInfo[i].cCode,
          				 pCameraInfoList->stCameraBriefInfo[i].cName,
				       pCameraInfoList->stCameraBriefInfo[i].cDevIp,
					 pCameraInfoList->stCameraBriefInfo[i].cVendorType,
					 pCameraInfoList->stCameraBriefInfo[i].cDevModelType
                               );
           } 
       } 
     printf("get camera list finished......%d\n",uiNum);
     if (uiNum>=1){
          printf("prepare data ...\n");
       //read image to buffer
         FILE *fp = NULL; 
         int iLen1 = -1;
         int iLen2 = -1;
         char *file1Buf=NULL;
	   char *file2Buf=NULL;
                   
         fp = fopen("1.jpg","rb");
         if (fp)  {
               fseek(fp,0,SEEK_END);
                   iLen1 = ftell(fp);
                   fseek(fp,0,SEEK_SET);
                   file1Buf = new char[iLen1];
                    CHECK_POINTER_VOID(file1Buf)
                   fread(file1Buf,iLen1,1,fp);
                    fclose(fp);
                   printf("load 1.jpg ok...=%d\n",iLen1); 
           }

         fp = fopen("2.jpg","rb");
         if (fp)  {
               fseek(fp,0,SEEK_END);
                   iLen2 = ftell(fp);
                   fseek(fp,0,SEEK_SET);
                   file2Buf = new char[iLen2];
                    CHECK_POINTER_VOID(file2Buf)
                   fread(file2Buf,iLen2,1,fp);
                   fclose(fp);
			printf("load 2.jpg ok...=%d\n",iLen2);
           }
            
         //upload photo 
         IVS_UINT32 uiSnapImgInfoLen = sizeof(IVS_IMG_UPLOAD_LIST) + sizeof(IVS_IMG_UPLOAD_INFO) ;//+ iLen1+iLen2;
         char * pSnapImgInfo = new char[uiSnapImgInfoLen];
         CHECK_POINTER_VOID(pSnapImgInfo)
         memset(pSnapImgInfo,0,sizeof(IVS_IMG_UPLOAD_LIST) + sizeof(IVS_IMG_UPLOAD_INFO));
         //memncpy(pSnapImgInfo+sizeof(IVS_IMG_UPLOAD_LIST) + sizeof(IVS_IMG_UPLOAD_INFO),file1Buf,iLen1);
         //memncpy(pSnapImgInfo+sizeof(IVS_IMG_UPLOAD_LIST) + sizeof(IVS_IMG_UPLOAD_INFO)+iLen1,file2Buf,iLen2);

         IVS_IMG_UPLOAD_LIST* pImgUploadList = (IVS_IMG_UPLOAD_LIST*)pSnapImgInfo;
	   strcpy(pImgUploadList->cCameraCode,pCameraInfoList->stCameraBriefInfo[0].cCode);
         pImgUploadList->iPicNum = 2;
         std::string g_picUrl = "http://zyong75.uicp.io/sample/test_upload";


         for (int i=0;i<2;i++){
	        pImgUploadList->stImgInfo[i].stMetaInfo.ullSnapTime = 20191101072130666 ;
              pImgUploadList->stImgInfo[i].stMetaInfo.eITSType = (i==0)?ITS_PICTURE_TYPE_FACE_SCENE:ITS_PICTURE_TYPE_PLATE;              		  pImgUploadList->stImgInfo[i].stMetaInfo.uiSnapshotGroup = 0;//抓拍组别
              memcpy(pImgUploadList->stImgInfo[i].stImgPath.clsvUrl,  g_picUrl.c_str(), g_picUrl.size());
		  //用于存放图片内容
	        pImgUploadList->stImgInfo[i].stImgData.uiPicLen= (i==0)?iLen1:iLen2;
		  pImgUploadList->stImgInfo[i].stImgData.pPictureBuf = (i==0)?file1Buf:file2Buf;
		  //pImgUploadList->stImgInfo[i].stImgData.uiPicBufLen

            } 
	IVS_IMG_EIGENVALUE pEigeValue;
	pEigeValue.eDataTag = IVS_SNAPSHOT_DATA_TAG_TYPE_UNKNOW;
	pEigeValue.iDataLen = 100;//featureLength;
	pEigeValue.pData = new char[100];//[featureLength];
	//memcpy_s(pEigeValue.pData, featureLength, featureValueBuf, featureLength); 
	memset(pEigeValue.pData,0xcc,100);

	IVS_EXTRA_INFO pExtraInfo;
      memset(&pExtraInfo,0,sizeof(pExtraInfo));
        printf("date prepare already finished! upload now...\n");
      iRet = IVS_SDK_UploadDeviceImgV2(iSessionID,pSnapImgInfo,&pExtraInfo, &pEigeValue,
                                    IVS_SNAPSHOT_TYPE_FULL_VEHICLE);
 
          if (IVS_SUCCEED != iRet)
		 printf("faild!  ecode=%d\n",iRet);
          else{
              printf("succeed!\n");
                for (int i=0;i<2;i++){
			printf("%d: %s\t%s\t%s\n",i,
				pImgUploadList->stImgInfo[i].stImgPath.cVcnHttpUrl,
                         pImgUploadList->stImgInfo[i].stImgPath.cVcnHttpsUrl,
                         pImgUploadList->stImgInfo[i].stImgPath.cUniPicID);
			} 
             }
          delete file1Buf;
             delete file2Buf;
             delete pSnapImgInfo;
             delete pEigeValue.pData ;
      }
       
     delete[] pCameraInfoList;//释放内存 
     pCameraInfoList = NULL;
        DeviceList.clear();      
    
     iRet = IVS_SDK_Logout(iSessionID);
     if (IVS_SUCCEED == iRet){
          printf("logout succeed!\n");
       }
     else{
          printf("logout faild! ecode=%d\n",iRet);
       }
   }
   else{
       printf("login faild! ecode=%i\n",iRet);
   }
   iRet = IVS_SDK_Cleanup(); 
   printf("cleanup=%i\n",iRet);
}
