//
//  MallocCollector.cpp
//  cfunction
//
//  Created by hzduanjiashun on 2018/3/27.
//  Copyright © 2018年 Daniel. All rights reserved.
//

#include <malloc/malloc.h>
#include <pthread.h>
#include <vector>
#include <string>
#include "ThreadLocal.hpp"
#include "MallocStorage.hpp"
#include "MallocCollector.hpp"
#include "MallocWrapZone.hpp"
#include "MemoryDetector.hpp"

namespace MD {
    int64_t CollectorChecker::max() {
        return MemoryDetector::GetInstance()->max_data_count();
    }
    void CollectorChecker::overflow(MemoryStorage* storage) {
        MemoryDetector::GetInstance()->DataOverflowNotifaction();
    }
    
    namespace Wrap {
        static pthread_key_t key_;
        
        pthread_key_t key() {
            return key_;
        }
        
        void deallocate(void *p) {
            MemoryStorage *s = reinterpret_cast<MemoryStorage *>(p);
            s->end();
            
            MemoryDetector::GetInstance()->pool().Detatch(s);
        }
        
        __attribute__((constructor))
        void make_storage_key() {
            pthread_key_create(&key_, deallocate);
        }
        
        struct Constructor {
            MemoryStorage* operator()() {
                auto p = new MemoryStorage();
                
                char buf[128] = {0};
                pthread_getname_np(pthread_self(), buf, sizeof(buf));
                VMAllocator<char> allocator;
                std::string thread_name(buf, allocator);
                p->setName(std::move(thread_name));
                
                MemoryDetector::GetInstance()->pool().Attach(p);
                return p;
            }
        };
        
        class ThreadStore : public ThreadLocal<MemoryStorage, Constructor> {
        public:
            ThreadStore() : ThreadLocal<MemoryStorage, Constructor>(key()) {}
        };
        
        void *malloc(malloc_zone_t *zone, size_t size) {
            malloc_zone_t* orig_zone = MemoryDetector::GetInstance()->wrap_zones().orig_zone_value(zone);
            if (MemoryDetector::GetInstance()->isEnabled()) {
                ThreadStore s;
                MemoryCollector c(s.value());
                c.set(zone, Action::Malloc, size);
                void *p = orig_zone->malloc(zone, size);
                
                return p;
            }
            else {
                void *p = orig_zone->malloc(zone, size);
                
                return p;
            }
        }
    }
}
