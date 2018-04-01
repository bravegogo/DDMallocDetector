//
//  MallocWrapZone.hpp
//  cfunction
//
//  Created by hzduanjiashun on 2018/3/27.
//  Copyright © 2018年 Daniel. All rights reserved.
//

#ifndef MallocWrapZone_hpp
#define MallocWrapZone_hpp

#include <stdio.h>
#include <malloc/malloc.h>
#include <vector>
#include "Allocator.hpp"

namespace MD {
    class MallocWrapZones {
    public:
        
        MallocWrapZones() {
            wrap_zones_.reserve(8);
        };
        
        void WrapZone(malloc_zone_t* zone) {
            MallocWrapZone z(zone);
            z.Wrap();
            wrap_zones_.emplace_back(z);
        }
        void UnWrapZone(malloc_zone_t* zone) {
            auto z = FindWrapZone(zone);
            z->UnWrap();
            wrap_zones_.erase(z);
        }
        
        malloc_zone_t* orig_zone_value(malloc_zone_t* zone) {
            auto z = FindWrapZone(zone);
            return z->orig_zone();
        }
        
    private:
        class MallocWrapZone {
        public:
            MallocWrapZone(malloc_zone_t *source) : orig_zone_(*source), orig_zone_ptr_(source) { };
            
            malloc_zone_t *orig_zone_ptr() { return orig_zone_ptr_; }
            malloc_zone_t *orig_zone() { return &orig_zone_; }
            
            void Wrap();
            void UnWrap();
            
        private:
            malloc_zone_t *orig_zone_ptr_;
            malloc_zone_t orig_zone_;
        };
        
        MallocWrapZones(MallocWrapZones &);
        MallocWrapZones& operator= (MallocWrapZones &);
        
        std::vector<MallocWrapZone>::iterator FindWrapZone(malloc_zone_t *z) {
            auto iz = std::find_if(wrap_zones_.begin(), wrap_zones_.end(), [z](MallocWrapZone& zone){
                return zone.orig_zone_ptr() == z;
            });
            return iz;
        }
        
        
        // var
        std::vector<MallocWrapZone, MD::VMAllocator<MallocWrapZone>> wrap_zones_;
    };
    
}


//class std::allocator<MallocWrapZone> {
//public:
//    pointer allocate(size_type __n, allocator<void>::const_pointer = 0) noexcept
//    {
//        size_type n = round_page(__n);
//        vm_address_t addr;
//        vm_allocate(mach_task_self(), &addr, n, PROT_WRITE|PROT_READ);
//        return reinterpret_cast<pointer>(addr);
//    }
//    void deallocate(pointer __p, size_type __n) noexcept
//    {
//        size_type n = round_page(__n);
//        vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(__p), n);
//    }
//};
//std::allocator<MD::MallocWrapZones>::pointer
//std::allocator<MD::MallocWrapZones>::allocate(size_type __n) {
//    
//    
//}

#endif /* MallocWrapZone_hpp */