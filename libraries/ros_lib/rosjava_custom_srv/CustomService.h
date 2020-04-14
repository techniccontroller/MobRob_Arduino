#ifndef _ROS_SERVICE_CustomService_h
#define _ROS_SERVICE_CustomService_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace rosjava_custom_srv
{

static const char CUSTOMSERVICE[] = "rosjava_custom_srv/CustomService";

  class CustomServiceRequest : public ros::Msg
  {
    public:
      typedef int32_t _size_type;
      _size_type size;

    CustomServiceRequest():
      size(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_size;
      u_size.real = this->size;
      *(outbuffer + offset + 0) = (u_size.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_size.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_size.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_size.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->size);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_size;
      u_size.base = 0;
      u_size.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_size.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_size.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_size.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->size = u_size.real;
      offset += sizeof(this->size);
     return offset;
    }

    const char * getType(){ return CUSTOMSERVICE; };
    const char * getMD5(){ return "97da5de9999c5ce84f539773c8d0b2a3"; };

  };

  class CustomServiceResponse : public ros::Msg
  {
    public:
      uint32_t res_length;
      typedef int64_t _res_type;
      _res_type st_res;
      _res_type * res;

    CustomServiceResponse():
      res_length(0), res(NULL)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->res_length >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->res_length >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->res_length >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->res_length >> (8 * 3)) & 0xFF;
      offset += sizeof(this->res_length);
      for( uint32_t i = 0; i < res_length; i++){
      union {
        int64_t real;
        uint64_t base;
      } u_resi;
      u_resi.real = this->res[i];
      *(outbuffer + offset + 0) = (u_resi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_resi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_resi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_resi.base >> (8 * 3)) & 0xFF;
      *(outbuffer + offset + 4) = (u_resi.base >> (8 * 4)) & 0xFF;
      *(outbuffer + offset + 5) = (u_resi.base >> (8 * 5)) & 0xFF;
      *(outbuffer + offset + 6) = (u_resi.base >> (8 * 6)) & 0xFF;
      *(outbuffer + offset + 7) = (u_resi.base >> (8 * 7)) & 0xFF;
      offset += sizeof(this->res[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t res_lengthT = ((uint32_t) (*(inbuffer + offset))); 
      res_lengthT |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1); 
      res_lengthT |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2); 
      res_lengthT |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3); 
      offset += sizeof(this->res_length);
      if(res_lengthT > res_length)
        this->res = (int64_t*)realloc(this->res, res_lengthT * sizeof(int64_t));
      res_length = res_lengthT;
      for( uint32_t i = 0; i < res_length; i++){
      union {
        int64_t real;
        uint64_t base;
      } u_st_res;
      u_st_res.base = 0;
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 3))) << (8 * 3);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 4))) << (8 * 4);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 5))) << (8 * 5);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 6))) << (8 * 6);
      u_st_res.base |= ((uint64_t) (*(inbuffer + offset + 7))) << (8 * 7);
      this->st_res = u_st_res.real;
      offset += sizeof(this->st_res);
        memcpy( &(this->res[i]), &(this->st_res), sizeof(int64_t));
      }
     return offset;
    }

    const char * getType(){ return CUSTOMSERVICE; };
    const char * getMD5(){ return "c12d00e29c21012e115751ce4f21977a"; };

  };

  class CustomService {
    public:
    typedef CustomServiceRequest Request;
    typedef CustomServiceResponse Response;
  };

}
#endif
