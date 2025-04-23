#ifndef __WEBSERVER_BASEREQUEST_H
#define __WEBSERVER_BASEREQUEST_H

class BaseRequest {
public:
    virtual ~BaseRequest() {}
    virtual void process() = 0;
};

#endif
