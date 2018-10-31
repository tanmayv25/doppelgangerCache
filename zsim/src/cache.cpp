/** $lic$
 * Copyright (C) 2012-2015 by Massachusetts Institute of Technology
 * Copyright (C) 2010-2013 by The Board of Trustees of Stanford University
 *
 * This file is part of zsim.
 *
 * zsim is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2.
 *
 * If you use this software in your research, we request that you reference
 * the zsim paper ("ZSim: Fast and Accurate Microarchitectural Simulation of
 * Thousand-Core Systems", Sanchez and Kozyrakis, ISCA-40, June 2013) as the
 * source of the simulator in any publications that use this software, and that
 * you send us a citation of your work.
 *
 * zsim is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "cache.h"
#include "hash.h"

#include "event_recorder.h"
#include "timing_event.h"
#include "zsim.h"

Cache::Cache(uint32_t _numLines, CC* _cc, CacheArray* _array, ReplPolicy* _rp, uint32_t _accLat, uint32_t _invLat, const g_string& _name)
    : cc(_cc), cc_dpg(NULL), array(_array), array_dpg(NULL), rp(_rp), rp_dpg_t(NULL), rp_dpg_d(NULL), accLat(_accLat), invLat(_invLat), name(_name) {}


Cache::Cache(uint32_t numLines, CC* _cc, CC* _cc_dpg, CacheArray* _array, DPG_Array* _array_dpg, ReplPolicy* _rp, ReplPolicy* _rp_dpg_t, ReplPolicy* _rp_dpg_d, uint32_t _accLat, uint32_t _invLat, const g_string& _name)
    : cc(_cc), cc_dpg(_cc_dpg), array(_array), array_dpg(_array_dpg), rp(_rp), rp_dpg_t(_rp_dpg_t), rp_dpg_d(_rp_dpg_d), accLat(_accLat), invLat(_invLat), name(_name) {}

const char* Cache::getName() {
    return name.c_str();
}

void Cache::setParents(uint32_t childId, const g_vector<MemObject*>& parents, Network* network) {
    cc->setParents(childId, parents, network);
//    if(cc_dpg != NULL)
//        cc_dpg->setParents(childId, parents, network);
}

void Cache::setChildren(const g_vector<BaseCache*>& children, Network* network) {
    cc->setChildren(children, network);
//    if(cc_dpg != NULL)
//        cc_dpg->setChildren(children, network);
}

void Cache::initStats(AggregateStat* parentStat) {
    AggregateStat* cacheStat = new AggregateStat();
    cacheStat->init(name.c_str(), "Cache stats");
    initCacheStats(cacheStat);
    parentStat->append(cacheStat);
}

bool Cache::isApprox(const Address lineAddr) {
    uint32_t *value = (uint32_t *)(lineAddr << lineBits);
    Address addr = (Address) value;
    e_approx_t approxType = no_approx;
    g_string approxString;
    approx_region_t * approxRegion = (approx_region_t *) approxInfo.find(approxTree23, (void *) addr);
    if (approxRegion && (addr >= (Address) approxRegion->addr_start) && (addr < (Address) approxRegion->addr_end)) {
        approxType = approxRegion->approx_type;
    }
    if( approxType == no_approx) {
        return false;
    } else {
        return true;
    }
    
}



void Cache::initCacheStats(AggregateStat* cacheStat) {
<<<<<<< HEAD
    if (cc_dpg != nullptr)
        cc_dpg->initStats(cacheStat);
    cc->initStats(cacheStat);
    
    if (array_dpg != nullptr)
        array_dpg->initStatsDPG(cacheStat);
=======
//    if (cc_dpg != nullptr)
//        cc_dpg->initStats(cacheStat);
    cc->initStats(cacheStat);
    
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
    array->initStats(cacheStat);

    rp->initStats(cacheStat);
    if (cc_dpg != nullptr) {
        rp_dpg_t->initStats(cacheStat);
        rp_dpg_d->initStats(cacheStat);
    }
}
<<<<<<< HEAD

// Tanmay: The access function in timing_cache.cpp should reflect the same changes
uint64_t Cache::access(MemReq& req) {
    
    uint64_t respCycle = req.cycle;
    // The access should be routed to DoppelGanger Cache if the requested address belongs to approximated data
    if(isApprox(req.lineAddr) && cc_dpg != NULL) {
=======
// Tanmay: Might be the changes should be copied as it is in timing_cache.cpp as well
uint64_t Cache::access(MemReq& req) {
    //Need to route access to DoppelGanger :
    
    uint64_t respCycle = req.cycle;

    if(isApprox(req.lineAddr) && cc_dpg != NULL) {
//      if(0) {
//        info("Shouldn't come here!!!");
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
        // Tanmay: Go to DG
        bool skipAccess = cc->startAccess(req); //may need to skip access due to races (NOTE: may change req.type!)
        if (likely(!skipAccess)) {
            bool updateReplacement = (req.type == GETS) || (req.type == GETX);
            int32_t lineId = array_dpg->lookup(req.lineAddr, &req, updateReplacement, cc, respCycle);
<<<<<<< HEAD
            info("Came out!!!");
=======
            //info("Came out!!!");
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
            respCycle += accLat;

            if (lineId == -1 && cc->shouldAllocate(req)) {
                //Make space for new line
                //Tanmay : ToDo : keep evicting till all tags are evicted
<<<<<<< HEAD
               
                // Tanmay: The cache evictions are sent internal to preinsert for evicting multiple tags in case
                // a data_line shared among various tag_lines gets evicted
                lineId = array_dpg->preinsert(req.lineAddr, &req, cc, respCycle); //find the lineId to replace
                // Tanmay: Now Do the Actual insertion
                // Do the actual insertion. NOTE: Now we must split insert into a 2-phase thing because cc unlocks us.
                array_dpg->postinsert(req.lineAddr, &req, lineId); 
=======
                //do {
                    // Tanmay: The cache evictions are sent internal to preinsert for evicting multiple tags in case
                    // a data_line shared among various tag_lines gets evicted
                lineId = array_dpg->preinsert(req.lineAddr, &req, cc, respCycle); //find the lineId to replace
                //trace(Cache, "[%s] Evicting 0x%lx", "DoppelGanger LLC", wbLineAddr);

                //Evictions are not in the critical path in any sane implementation -- we do not include their delays
                //NOTE: We might be "evicting" an invalid line for all we know. Coherence controllers will know what to do
                //cc_dpg->processEviction(req, wbLineAddr, lineId, respCycle); //1. if needed, send invalidates/downgrades to lower level
                //} while(isMultiple == true);
                // Tanmay: Evictions completed

                // Tanmay: Now Do the Actual insertion
                array_dpg->postinsert(req.lineAddr, &req, lineId); //do the actual insertion. NOTE: Now we must split insert into a 2-phase thing because cc unlocks us.
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
            }
            // Enforce single-record invariant: Writeback access may have a timing
            // record. If so, read it.
            EventRecorder* evRec = zinfo->eventRecorders[req.srcId];
            TimingRecord wbAcc;
            wbAcc.clear();
            if (unlikely(evRec && evRec->hasRecord())) {
                wbAcc = evRec->popRecord();
            }

            respCycle = cc->processAccess(req, lineId, respCycle);

            // Access may have generated another timing record. If *both* access
            // and wb have records, stitch them together
            if (unlikely(wbAcc.isValid())) {
                if (!evRec->hasRecord()) {
                    // Downstream should not care about endEvent for PUTs
                    wbAcc.endEvent = nullptr;
                    evRec->pushRecord(wbAcc);
                } else {
                    // Connect both events
                    TimingRecord acc = evRec->popRecord();
                    assert(wbAcc.reqCycle >= req.cycle);
                    assert(acc.reqCycle >= req.cycle);
                    DelayEvent* startEv = new (evRec) DelayEvent(0);
                    DelayEvent* dWbEv = new (evRec) DelayEvent(wbAcc.reqCycle - req.cycle);
                    DelayEvent* dAccEv = new (evRec) DelayEvent(acc.reqCycle - req.cycle);
                    startEv->setMinStartCycle(req.cycle);
                    dWbEv->setMinStartCycle(req.cycle);
                    dAccEv->setMinStartCycle(req.cycle);
                    startEv->addChild(dWbEv, evRec)->addChild(wbAcc.startEvent, evRec);
                    startEv->addChild(dAccEv, evRec)->addChild(acc.startEvent, evRec);

                    acc.reqCycle = req.cycle;
                    acc.startEvent = startEv;
                    // endEvent / endCycle stay the same; wbAcc's endEvent not connected
                    evRec->pushRecord(acc);
                }
            }
        }

        cc->endAccess(req);

        assert_msg(respCycle >= req.cycle, "[%s] resp < req? 0x%lx type %s childState %s, respCycle %ld reqCycle %ld",
                "DoppelGanger LLC", req.lineAddr, AccessTypeName(req.type), MESIStateName(*req.state), respCycle, req.cycle);
        return respCycle;
    } else {
        bool skipAccess = cc->startAccess(req); //may need to skip access due to races (NOTE: may change req.type!)
        if (likely(!skipAccess)) {
            bool updateReplacement = (req.type == GETS) || (req.type == GETX);
            int32_t lineId = array->lookup(req.lineAddr, &req, updateReplacement);
            respCycle += accLat;

            if (lineId == -1 && cc->shouldAllocate(req)) {
                //Make space for new line
                Address wbLineAddr;
                lineId = array->preinsert(req.lineAddr, &req, &wbLineAddr); //find the lineId to replace
                trace(Cache, "[%s] Evicting 0x%lx", name.c_str(), wbLineAddr);

                //Evictions are not in the critical path in any sane implementation -- we do not include their delays
                //NOTE: We might be "evicting" an invalid line for all we know. Coherence controllers will know what to do
                cc->processEviction(req, wbLineAddr, lineId, respCycle); //1. if needed, send invalidates/downgrades to lower level

                array->postinsert(req.lineAddr, &req, lineId); //do the actual insertion. NOTE: Now we must split insert into a 2-phase thing because cc unlocks us.
            }
            // Enforce single-record invariant: Writeback access may have a timing
            // record. If so, read it.
            EventRecorder* evRec = zinfo->eventRecorders[req.srcId];
            TimingRecord wbAcc;
            wbAcc.clear();
            if (unlikely(evRec && evRec->hasRecord())) {
                wbAcc = evRec->popRecord();
            }

            respCycle = cc->processAccess(req, lineId, respCycle);

            // Access may have generated another timing record. If *both* access
            // and wb have records, stitch them together
            if (unlikely(wbAcc.isValid())) {
                if (!evRec->hasRecord()) {
                    // Downstream should not care about endEvent for PUTs
                    wbAcc.endEvent = nullptr;
                    evRec->pushRecord(wbAcc);
                } else {
                    // Connect both events
                    TimingRecord acc = evRec->popRecord();
                    assert(wbAcc.reqCycle >= req.cycle);
                    assert(acc.reqCycle >= req.cycle);
                    DelayEvent* startEv = new (evRec) DelayEvent(0);
                    DelayEvent* dWbEv = new (evRec) DelayEvent(wbAcc.reqCycle - req.cycle);
                    DelayEvent* dAccEv = new (evRec) DelayEvent(acc.reqCycle - req.cycle);
                    startEv->setMinStartCycle(req.cycle);
                    dWbEv->setMinStartCycle(req.cycle);
                    dAccEv->setMinStartCycle(req.cycle);
                    startEv->addChild(dWbEv, evRec)->addChild(wbAcc.startEvent, evRec);
                    startEv->addChild(dAccEv, evRec)->addChild(acc.startEvent, evRec);

                    acc.reqCycle = req.cycle;
                    acc.startEvent = startEv;
                    // endEvent / endCycle stay the same; wbAcc's endEvent not connected
                    evRec->pushRecord(acc);
                }
            }
        }

        cc->endAccess(req);

        assert_msg(respCycle >= req.cycle, "[%s] resp < req? 0x%lx type %s childState %s, respCycle %ld reqCycle %ld",
                name.c_str(), req.lineAddr, AccessTypeName(req.type), MESIStateName(*req.state), respCycle, req.cycle);
        return respCycle;
    }
}

void Cache::startInvalidate() {
<<<<<<< HEAD
//    cc_dpg->startInv(); //Finally they are a unified cache so cc will take care of invalidations to the upper hierarchy
=======
//    cc_dpg->startInv(); //Finally they are a unified cache
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
    cc->startInv(); //note we don't grab tcc; tcc serializes multiple up accesses, down accesses don't see it
}

uint64_t Cache::finishInvalidate(const InvReq& req) {

    if(isApprox(req.lineAddr) && array_dpg != NULL) {
<<<<<<< HEAD
      if(1) {
=======
//    if(0) {
>>>>>>> 1b74717857236d341e3b779ff6cfd2a5109d5425
        int32_t lineId = array_dpg->lookup(req.lineAddr, nullptr, false,  nullptr, 0);
        assert_msg(lineId != -1, "[%s] Invalidate on non-existing address 0x%lx type %s lineId %d, reqWriteback %d", "DoppelGanger LLC", req.lineAddr, InvTypeName(req.type), lineId, *req.writeback);
        trace(Cache, "[%s] Invalidate start 0x%lx type %s lineId %d, reqWriteback %d", "DoppelGanger LLC", req.lineAddr, InvTypeName(req.type), lineId, *req.writeback);
        uint64_t respCycle = req.cycle + invLat;
        respCycle = cc->processInv(req, lineId, respCycle); //send invalidates or downgrades to children, and adjust our own state
        trace(Cache, "[%s] Invalidate end 0x%lx type %s lineId %d, reqWriteback %d, latency %ld", "DoppelGanger LLC", req.lineAddr, InvTypeName(req.type), lineId, *req.writeback, respCycle - req.cycle);
        return respCycle;
    } else {
        int32_t lineId = array->lookup(req.lineAddr, nullptr, false);
        assert_msg(lineId != -1, "[%s] Invalidate on non-existing address 0x%lx type %s lineId %d, reqWriteback %d", name.c_str(), req.lineAddr, InvTypeName(req.type), lineId, *req.writeback);
        uint64_t respCycle = req.cycle + invLat;
        trace(Cache, "[%s] Invalidate start 0x%lx type %s lineId %d, reqWriteback %d", name.c_str(), req.lineAddr, InvTypeName(req.type), lineId, *req.writeback);
        respCycle = cc->processInv(req, lineId, respCycle); //send invalidates or downgrades to children, and adjust our own state
        trace(Cache, "[%s] Invalidate end 0x%lx type %s lineId %d, reqWriteback %d, latency %ld", name.c_str(), req.lineAddr, InvTypeName(req.type), lineId, *req.writeback, respCycle - req.cycle);

        return respCycle;
    }
}
