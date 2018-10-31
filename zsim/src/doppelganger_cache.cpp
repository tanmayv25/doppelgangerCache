#include "cache_arrays.h"
#include "hash.h"
#include "repl_policies.h"
#include "zsim.h"
#include "math.h"
#include "limits.h"
#include "float.h"
#include "event_recorder.h"
#include "timing_event.h"

#define DATA_INT_MIN INT_MIN
#define DATA_INT_MAX INT_MAX

#define DATA_FLOAT_MIN -FLT_MAX
#define DATA_FLOAT_MAX FLT_MAX

//Tanmay: The Mbits number of bits in the Map value. They must be changed for analysis. The paper compares 12, 13, 14. Higher Mbits, lesser is the approximation exploitation
#define Mbits   12

#define DATA_SCALE 2


/* DoppelGanger array implementation */
DPG_Array::DPG_Array(uint32_t _numLines, uint32_t _assoc, ReplPolicy* _rp_dpg_t, ReplPolicy* _rp_dpg_d, HashFamily* _hf, int _imin, int _imax, float _fmin, float _fmax) : rp_dpg_t(_rp_dpg_t), rp_dpg_d(_rp_dpg_d), hf(_hf), numLines(_numLines), assoc(_assoc), IMIN(_imin), IMAX(_imax), FMIN(_fmin), FMAX(_fmax)  {
    //info("DoppelGanger Creation begins");
    array_t = gm_calloc<Tag>(numLines);
    array_d = gm_calloc<Data>(numLines/DATA_SCALE);
    // Set all pointers to -1
    numSets_t = numLines/assoc;
    numSets_d = numSets_t/DATA_SCALE;
    setMask_t = numSets_t - 1;
    assert_msg(isPow2(numSets_t), "must have a power of 2 # sets, but you specified %d", numSets_t);
    setMask_d = numSets_d - 1;
    assert_msg(isPow2(numSets_d), "must have a power of 2 # sets, but you specified %d", numSets_d);
    
    /* Initializes the data structures to point nowhere */
    for (uint32_t data_id = 0; data_id < numLines/DATA_SCALE ; data_id++) {
            array_d[data_id].map_value = -1;
            array_d[data_id].tag_head_id = -1;
    }

    for (uint32_t tag_id = 0; tag_id < numLines ; tag_id++) {
            array_t[tag_id].next = -1;
            array_t[tag_id].prev = -1;
            array_t[tag_id].addr = -1;
            array_t[tag_id].data_id = -1;
    }
    //info("DoppelGanger Creation ends");
}


/* This function removes the mentioned tag_id from its current LinkedList and returns the previous member */
void DPG_Array::remove_tag(int32_t tag_id) {
    //info("Tag removed %d!!!", tag_id);
//    int32_t new_head; // = array_t[tag_id].prev;
    //Make changes to the list
    if(array_t[tag_id].prev != -1) {
        if(array_t[tag_id].next != -1) {
            array_t[array_t[tag_id].prev].next = array_t[tag_id].next;
            array_t[array_t[tag_id].next].prev = array_t[tag_id].prev;
        } else {
            array_t[array_t[tag_id].prev].next = -1;
            array_d[array_t[tag_id].data_id].tag_head_id = array_t[tag_id].prev;
        }
    } else {
        if(array_t[tag_id].next != -1) {
            array_t[array_t[tag_id].next].prev = -1;
        } else {
            array_d[array_t[tag_id].data_id].map_value = -1;
            array_d[array_t[tag_id].data_id].tag_head_id = -1;
            // No updates in the linked list required as it is the only element
        }
    }

    // Clearing OFF
    array_t[tag_id].next = -1;
    array_t[tag_id].prev = -1;
    array_t[tag_id].addr = -1;
    array_t[tag_id].data_id = -1;

}


/* Adds the tag_id to the list pointed by the head_id making it the new head of the list */
void DPG_Array::add_tag(int32_t tag_id, int32_t data_id) {
    //info("Shouldn't come here!!!");
    int32_t head_id = array_d[data_id].tag_head_id;
    if(head_id != -1) {
    array_t[tag_id].prev = head_id;
    array_t[head_id].next = tag_id;
    }
    array_t[tag_id].data_id = data_id;
    
    //update the head ptr in data_line to point to the newly added head
    array_d[data_id].tag_head_id = tag_id;
}

/* This function searches for the data_id with the similar map passed */
int32_t DPG_Array::lookup_data(const int32_t map) {
    uint32_t set_d = hf->hash(0, map) & setMask_d;
    int32_t first_d = set_d*assoc;
    for (int32_t data_id = first_d; data_id < (int32_t) (first_d + assoc); data_id++) {
        if ((array_d[data_id].map_value ==  map) && (array_d[data_id].tag_head_id != -1)) {
            //info("data_id : %d map value: %d tag_ptr: %d in_map: %d", data_id, array_d[data_id].map_value, array_d[data_id].tag_head_id, map);
            return data_id;
        }
    }
   
    return -1;
}

/* This function is called to look for any unoccupied data line in the data cache. It returns -1 when no free data line is found */
int32_t DPG_Array::get_free_data(const int32_t map, CC* cc_dpg, const MemReq* req, uint64_t respCycle) {
    
    
    uint32_t set_d = hf->hash(0, map) & setMask_d;
    int32_t first_d = set_d*assoc;
    int32_t data_id;

    int32_t i; uint64_t count = 0;
//    for (i = 0; i < numLines; i++) {
//        if (array_t[i].data_id == -1) {
//            count++;
//        }
//    }

//    info("portion free in tag cache : %f", (float)((float)count/(float)numLines));
    
    count = 0;
    for (i = first_d; i < (int32_t)(first_d + assoc); i++) {
        if (array_d[i].tag_head_id == -1) {
            count++;
        }
    }

//    info("free portion in data cache : %f", (float)((float)count/(float)assoc));
    for (data_id = first_d; data_id < (int32_t) (first_d + assoc); data_id++) {
        if (array_d[data_id].tag_head_id == -1) {
            return data_id;
        }
    }

    //info("About to rank data_id"); 
    data_id = rp_dpg_d->rankCands(req, SetAssocCands(first_d, first_d + assoc));
    //info("Ranked data_id"); 
    evict_data(data_id, cc_dpg, req, respCycle);

    return data_id;
}

/* This function is called to look for any unoccupied tag line in the tag cache array. If free is not found then evict the lines to create free */
int32_t DPG_Array::get_free_tag(const Address lineAddr, CC* cc_dpg, const MemReq* req, uint64_t respCycle) {
    uint32_t set_t = hf->hash(0, lineAddr) & setMask_t;
    int32_t first_t = set_t*assoc;
    int32_t tag_id;
    
    for (tag_id = first_t; tag_id < (int32_t)(first_t + assoc); tag_id++) {
        if (array_t[tag_id].data_id == -1) {
            return tag_id;
        }
    }
    
    tag_id = rp_dpg_t->rankCands(req, SetAssocCands(first_t, first_t + assoc));
    evict_tag(tag_id, cc_dpg, req, respCycle);
    return tag_id;
}

/* This function will evict the data id and the associated tag ids */
void DPG_Array::evict_data(const int32_t data_id, CC* cc_dpg, const MemReq* req, uint64_t respCycle) {
    if(data_id == -1)
        return;
    assert_msg((data_id > -1 && data_id <(int32_t)(numLines/DATA_SCALE)), "Data id %d to be evicted is beyond the expected range in evict", data_id);
    do {
        int32_t tag_id = array_d[data_id].tag_head_id;
        evict_tag(tag_id, cc_dpg, req, respCycle);
    } while (array_d[data_id].tag_head_id != -1);
}

/* This function will evict the tag id */
void DPG_Array::evict_tag(const int32_t tag_id, CC* cc_dpg, const MemReq* req, uint64_t respCycle) {
    if(tag_id == -1)
        return;
    assert_msg((tag_id > -1 && tag_id < (int32_t)numLines), "Tag id %d to be evicted is beyond the expected range in evict", tag_id);
    trace(Cache, "[%s] Evicting 0x%lx", "DoppelGanger LLC", array_t[tag_id].addr);
    cc_dpg->processEviction(*req, array_t[tag_id].addr, (tag_id + numLines), respCycle);
    rp_dpg_t->replaced(tag_id);
    remove_tag(tag_id);

}


int32_t DPG_Array::lookup(const Address lineAddr, const MemReq* req, bool updateReplacement, CC* cc_dpg, uint64_t respCycle) {
    uint32_t set_t = hf->hash(0, lineAddr) & setMask_t;
    int32_t first_t = set_t*assoc;
    for (int32_t tag_id = first_t; tag_id < (int32_t)(first_t + assoc); tag_id++) {
        // Tanmay: Need to check whether both the data line and tag lines are present and didn't got evicted in presinsert earlier
        if ((array_t[tag_id].addr ==  lineAddr) && (array_t[tag_id].data_id != -1)) {
            int32_t map = get_map(lineAddr);
            if (map != array_d[array_t[tag_id].data_id].map_value) { //The map changed due to a write
            //if (updateReplacement && map != array_d[array_t[tag_id].data_id].map_value) { //The map changed due to a write
                // Remove the tag id from the linked list which was pointing to the previous block
                remove_tag(tag_id);
                // Search for already present data block with identical map. If present then no work needs to be done. Just update the tag id to the linked list 
                int32_t new_data = lookup_data(map);
                if (new_data != -1) { // The new map data block was already present
                    //memcpy((uint8_t *)(lineAddr << lineBits), (uint8_t *)(array_t[array_d[new_data].tag_head_id].addr << lineBits), 64);
                    //info("look_up :: look_data set value %d map %d", new_data, map);
                    add_tag(tag_id, new_data);
                } else { // Need to evict the data_id block to create space for the data line with new map value
                    new_data = get_free_data(map, cc_dpg, req, respCycle);
                    //info("lookup :: get_free_data set value %d map %d", new_data, map);
                    array_t[tag_id].data_id = new_data;
                    array_d[new_data].map_value = map;
                    array_d[new_data].tag_head_id = tag_id;
                }
  
            }
            // If the data id block was not found then the new data line should be brought in, else the tag_id is passed
            if(array_t[tag_id].data_id != -1) {
                if (updateReplacement) rp_dpg_t->update(tag_id, req);
                if (updateReplacement) rp_dpg_d->update(array_t[tag_id].data_id, req);
                return (tag_id + numLines); // + numLines to make CC happy
            }
        }
    }
    //info("Not here"); 
    return -1;
}


uint32_t DPG_Array::preinsert(const Address lineAddr, const MemReq* req, CC* cc_dpg, uint64_t respCycle) { //Tanmay: Need to evict here. May be no evictions are needed if evicted data_lines and tag_lines are present
    int32_t tag_id = get_free_tag(lineAddr, cc_dpg, req, respCycle);
    int32_t map = get_map(lineAddr);
    int32_t data_id = lookup_data(map);
    if(data_id != -1) {
        add_tag(tag_id, data_id);
        //info("preinsert :: look_data set value %d map %d", data_id, map);
    } else {
        data_id = get_free_data(map, cc_dpg, req, respCycle);
        array_t[tag_id].data_id = data_id; 
        array_d[data_id].map_value = map;
        array_d[data_id].tag_head_id = tag_id;
    } 
//    info("Should be 0 : %d", data_id);
    //info("Preinsert!!!");
    return (tag_id + numLines); // + numLines to make CC happy
}

void DPG_Array::postinsert(const Address lineAddr, const MemReq* req, uint32_t candidate_t) {
    candidate_t -= numLines;
    uint32_t candidate_d = array_t[candidate_t].data_id;
    rp_dpg_t->replaced(candidate_t);
    rp_dpg_d->replaced(candidate_d);
    

    array_t[candidate_t].addr = lineAddr;

    rp_dpg_t->update(candidate_t, req);
    rp_dpg_d->update(candidate_d, req);
    //info("Post insert!!!");
}


/* This function returns the map value for the data pointed to by the lineAddr */
int32_t DPG_Array::get_map(const Address lineAddr) {
        uint32_t hf_range;
        uint32_t hf_avg;
        uint32_t map;
        int *value = (int *)(lineAddr << lineBits);
        Address addr = (Address) value;
        e_approx_t approxType = no_approx;
        g_string approxString;
        approx_region_t * approxRegion = (approx_region_t *) approxInfo.find(approxTree23, (void *) addr);
        if (approxRegion && (addr >= (Address) approxRegion->addr_start) && (addr < (Address) approxRegion->addr_end)) {
            approxType = approxRegion->approx_type;
        }
        if(approxType == approx_uint) {
            int min, max, avg = 0; 
            int range;
            min = DATA_INT_MAX;
            max = DATA_INT_MIN;
            for(int i = 0; i < 16; i++) {
                if(value[i] < min)
                    min = value[i];
                if(value[i] > max)
                    max = value[i];
                avg = avg + value[i];
            }
            avg = avg / 16;
            range = max - min;
            hf_avg = (uint32_t)(((avg - IMIN) * ((1 << Mbits) - 1))/(IMAX - IMIN));
            hf_range = (uint32_t)(((range - IMIN) * ((1 << Mbits) - 1))/(IMAX - IMIN));
            
        } else { //Tanmay: Question: Assuming single-precision floating point
            float *value_f = (float *)(lineAddr << lineBits);
            float min, max, avg = 0;
            float range;
            min = DATA_FLOAT_MAX;
            max = DATA_FLOAT_MIN;
            for(int i = 0; i < 16; i++) {
                if(value_f[i] < min)
                    min = value_f[i];
                if(value_f[i] > max)
                    max = value_f[i];
                avg = avg + value_f[i];

            }
            range = max - min;
            avg = avg / 16;
            hf_avg = (uint32_t)(((avg - FMIN) * ((1 << Mbits) - 1))/(FMAX - FMIN));
            hf_range = (uint32_t)(((range - FMIN) * ((1 << Mbits) - 1))/(FMAX - FMIN));
        }

        // Tanmay: map the hash value to a map value
        map = ((hf_range >> (M/2)) << M) | hf_avg;
        //info("The map value is %d", map);
        return map;
}


