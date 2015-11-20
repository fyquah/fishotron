//
//  PageSizes.h
//  OpenCVHello
//
//  Created by Tom on 11/11/2015.
//  Copyright © 2015 Tom Hartley. All rights reserved.
//

#ifndef PageSizes_h
#define PageSizes_h

namespace fish {

typedef enum {
    A4 = 2,
    A3 = 3,
} thePageSize;
//^ not 100% sure that enum even needs to exist...?

typedef enum {
    PageHeight = 0, //mm
    PageWidth = 1, //mm
    FiducialWidth = 2, //mm
} PageDims;

static int pageSizes[][4] ={
        {-1,-1,-1}, //Unknown - Offset 0
        {144,227,20}, //A4 - Offset 1
        {-1,-1,-1}, //Unknown - Offset 0
        {-1,-1,-1} //Unknown - Offset 0
};

}


#endif /* PageSizes.h */
