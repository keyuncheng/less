#include "common/Config.hh" 
#include "ec/ECDAG.hh"
#include "ec/ECPolicy.hh"
#include "ec/ECBase.hh"
#include "ec/ECTask.hh"

#include "ec/RSCONV.hh"
#include "ec/HHXORPlus.hh"
#include "ec/HTEC.hh"
#include "ec/BUTTERFLY.hh"
#include "ec/Clay.hh"
#include "ec/ETRSConv.hh"
#include "ec/ETHTEC.hh"
#include "ec/ETHHXORPlus.hh"
#include "ec/LESS.hh"

#include <utility>

using namespace std;

void usage() {
    // printf("usage: ./CodeTest code_name n k w blockSizeBytes disk_seek_time_ms disk_bdwt_MBps failed_ids\n");
    printf("usage: ./CodeTest code_name n k w blockSizeBytes <failed_ids>\n");
}

double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1e+6 + (double)tv.tv_usec;
}

int main(int argc, char** argv) {

    if (argc < 7) {
        usage();
        return 0;
    }

    string codeName = argv[1];
    int n = atoi(argv[2]);
    int k = atoi(argv[3]);
    int w = atoi(argv[4]);
    unsigned long long blockSizeBytes = atoi(argv[5]);

    if (blockSizeBytes % (16 * w) != 0)
    {
        int nearestBS = (int) ceil(1.0 * blockSizeBytes / (16 * w)) * 16 * w;
        printf("Warning: block size must be a multiple of 16 * w = %d; rounded to the nearest: %d\n", 16 * w, nearestBS);
        blockSizeBytes = nearestBS;
    }
    
    unsigned long long pktSizeBytes = blockSizeBytes / w;

    string ecid = codeName + "_" + to_string(n) + "_" + to_string(k) + "_" + to_string(w);

    vector<int> failed_ids;
    for (int i = 6; i < argc; i++) {
        int failed_id = atoi(argv[i]);
        if (failed_id >= n) {
            cout << "error: failed_id " << failed_id << " is larger than n " << n << endl;
            return 1;
        }
        failed_ids.push_back(failed_id);
    }

    // double disk_seek_time_ms = stod(argv[5]);
    // double disk_bdwt_MBps = stod(argv[6]);
    
    string confpath = "./conf/sysSetting.xml";
    Config* conf = new Config(confpath);

    cout << "Loaded EC Schemes:" << endl;
    for (auto item : conf->_ecPolicyMap) {
        cout << item.first << endl;
    }

    ECPolicy* ecpolicy = conf->_ecPolicyMap[ecid];
    ECBase* ec = ecpolicy->createECClass();

    // field width (special setting for LESS)
    int fw = 8; // default fw
    if (codeName == "LESS") {
        uint32_t e, f;
        if (LESS::getAvailPrimElements(n, k, w, fw, e, f) == false) {
            cout << "LESS::getAvailPrimElements() failed to find primitive element" << endl;
            exit(1);
        }
    }

    int n_data_symbols = k * w;
    int n_code_symbols = (n - k) * w;

    // 0. prepare data buffers
    char **databuffers = (char **)calloc(n_data_symbols, sizeof(char *));
    for (int i = 0; i < n_data_symbols; i++) {
        databuffers[i] = (char *)calloc(pktSizeBytes, sizeof(char));
        char v = i;
        memset(databuffers[i], v, pktSizeBytes);
    }

    // 1. prepare code buffers
    char **codebuffers = (char **)calloc(n_code_symbols, sizeof(char *));
    for (int i = 0; i < n_code_symbols; i++) {
        codebuffers[i] = (char *)calloc(pktSizeBytes, sizeof(char));
        memset(codebuffers[i], 0, pktSizeBytes);
    }

    double initEncodeTime=0, initDecodeTime=0;
    double encodeTime = 0, decodeTime=0;
    initEncodeTime -= getCurrentTime();

    ECDAG* encdag = ec->Encode();
    vector<ECTask*> encodetasks;
    unordered_map<int, char*> encodeBufMap;
    vector<int> toposeq = encdag->toposort();
    for (int i=0; i<toposeq.size(); i++) {
        ECNode* curnode = encdag->getNode(toposeq[i]);
        curnode->parseForClient(encodetasks);
    }
    for (int i=0; i<n_data_symbols; i++) encodeBufMap.insert(make_pair(i, databuffers[i]));
    for (int i=0; i<n_code_symbols; i++) encodeBufMap.insert(make_pair(n_data_symbols+i, codebuffers[i]));
    initEncodeTime += getCurrentTime();
    initEncodeTime /= 1000000;

    // free list to support shortening
    vector<int> shortening_free_list;

    encodeTime -= getCurrentTime();
    for (int taskid = 0; taskid < encodetasks.size(); taskid++) {
        ECTask* compute = encodetasks[taskid];
        // compute->dump();

        vector<int> children = compute->getChildren();
        unordered_map<int, vector<int>> coefMap = compute->getCoefMap();
        int col = children.size();
        int row = coefMap.size();

        vector<int> targets;
        int* matrix = (int*)calloc(row*col, sizeof(int));
        char** data = (char**)calloc(col, sizeof(char*));
        char** code = (char**)calloc(row, sizeof(char*));
        for (int bufIdx=0; bufIdx<children.size(); bufIdx++) {
            int child = children[bufIdx];

            // create buffers to support shortening
            if (child >= n * w && encodeBufMap.find(child) == encodeBufMap.end()) {
                shortening_free_list.push_back(child);
                char* slicebuf = (char *) calloc(pktSizeBytes, sizeof(char));
                encodeBufMap[child] = slicebuf;
            }

            data[bufIdx] = encodeBufMap[child];
        }
        int codeBufIdx = 0;
        for (auto it: coefMap) {
            int target = it.first;
            char* codebuf; 
            if (encodeBufMap.find(target) == encodeBufMap.end()) {
                codebuf = (char *)calloc(pktSizeBytes, sizeof(char));
                encodeBufMap.insert(make_pair(target, codebuf));
            } else {
                codebuf = encodeBufMap[target];
            }
            code[codeBufIdx] = codebuf;
            targets.push_back(target);
            vector<int> curcoef = it.second;
            for (int j = 0; j < col; j++) {
                matrix[codeBufIdx * col + j] = curcoef[j];
            }
            codeBufIdx++;
        }
        Computation::Multi(code, data, matrix, row, col, pktSizeBytes, "Isal", fw);

        free(matrix);
        free(data);
        free(code);
    }

    // free buffers in shortening free list
    for (auto pkt_idx : shortening_free_list) {
        free(encodeBufMap[pkt_idx]);
    }
    shortening_free_list.clear();

    encodeTime += getCurrentTime();
    encodeTime /= 1000000;

    // debug encode
    for (int i=0; i<n_data_symbols; i++) {
        char* curbuf = (char*)databuffers[i];
        cout << "dataidx = " << i << ", value = " << (int)curbuf[0] << endl;
    }
    for (int i=0; i<n_code_symbols; i++) {
        char* curbuf = (char*)codebuffers[i];
        cout << "codeidx = " << n_data_symbols+i << ", value = " << (int)curbuf[0] << endl;
    }

    cout << "========================" << endl;

    // decode
    initDecodeTime -= getCurrentTime();

    ECPolicy* ecpolicy1 = conf->_ecPolicyMap[ecid];
    ECBase* ec1 = ecpolicy->createECClass();

    vector<int> failsymbols;
    unordered_map<int, char*> repairbuf;

    cout<< "failed nodes: ";
    for (auto failnode : failed_ids) {
        cout << failnode << " ";
        vector<int> failed_symbols_node = ec1->getNodeSubPackets(failnode);
        for (auto symbol : failed_symbols_node) {
            failsymbols.push_back(symbol);
        }
    }
    cout << endl;

    for(int i=0; i<failsymbols.size(); i++) {
        char* tmpbuf = (char*)calloc(pktSizeBytes, sizeof(char));
        repairbuf[failsymbols[i]] = tmpbuf;
    }

    vector<int> availsymbols;
    for (int i=0; i<n*w; i++) {
        if (find(failsymbols.begin(), failsymbols.end(), i) == failsymbols.end())
            availsymbols.push_back(i);
    }

    cout << "failed symbols: ";
    for (int i=0; i<failsymbols.size(); i++) {
        cout << failsymbols[i] << " ";
    }
    cout << endl;

    cout << "available symbols:";
    for(int i=0; i<availsymbols.size(); i++) {
        cout << availsymbols[i] << " ";
    }
    cout << endl;

    ECDAG* decdag = ec1->Decode(availsymbols, failsymbols);
    vector<ECTask*> decodetasks;
    unordered_map<int, char*> decodeBufMap;
    vector<int> dectoposeq = decdag->toposort();
    for (int i=0; i<dectoposeq.size(); i++) { 
        ECNode* curnode = decdag->getNode(dectoposeq[i]);
        curnode->parseForClient(decodetasks);
    }
    for (int i=0; i<n_data_symbols; i++) {
        if (find(failsymbols.begin(), failsymbols.end(), i) == failsymbols.end())
            decodeBufMap.insert(make_pair(i, databuffers[i]));
        else
            decodeBufMap.insert(make_pair(i, repairbuf[i]));
    }
    for (int i=0; i<n_code_symbols; i++) 
        if (find(failsymbols.begin(), failsymbols.end(), n_data_symbols+i) == failsymbols.end())
            decodeBufMap.insert(make_pair(n_data_symbols+i, codebuffers[i]));
        else
            decodeBufMap.insert(make_pair(i, repairbuf[n_data_symbols+i]));

    initDecodeTime += getCurrentTime();
    initDecodeTime /= 1000000;

    decodeTime -= getCurrentTime();

    /**
     * @brief record number of disk seeks and number of sub-packets to read for every node
     * @disk_read_pkts_map <node> sub-packets read in each node
     * @disk_read_info_map <node_id, <num_disk_seeks, num_pkts_read>>
     * 
     */
    map<int, vector<int>> disk_read_pkts_map;
    map<int, pair<int, int>> disk_read_info_map;

    int sum_packets_read = 0;
    double norm_repair_bandwidth = 0;

    // init the map
    for (int node_id = 0; node_id < n; node_id++) {
        disk_read_pkts_map[node_id].clear();
        disk_read_info_map[node_id] = make_pair(0, 0);
    }

    for (int taskid = 0; taskid < decodetasks.size(); taskid++) {
        ECTask* compute = decodetasks[taskid];
        compute->dump();

        vector<int> children = compute->getChildren();
        unordered_map<int, vector<int>> coefMap = compute->getCoefMap();
        int col = children.size();
        int row = coefMap.size();

        vector<int> targets;
        int* matrix = (int*)calloc(row*col, sizeof(int));
        char** data = (char**)calloc(col, sizeof(char*));
        char** code = (char**)calloc(row, sizeof(char*));
        for (int bufIdx=0; bufIdx<children.size(); bufIdx++) {
            int child = children[bufIdx];

            // create buffers to support shortening
            if (child >= n * w && decodeBufMap.find(child) == decodeBufMap.end()) {
                shortening_free_list.push_back(child);
                char* slicebuf = (char *) calloc(pktSizeBytes, sizeof(char));
                decodeBufMap[child] = slicebuf;
            }

            if (child < n * w) {
                int node_id = child / w;
                vector<int> &read_pkts = disk_read_pkts_map[node_id];
                if (find(read_pkts.begin(), read_pkts.end(), child) == read_pkts.end()) {
                    read_pkts.push_back(child);
                }
            }

            data[bufIdx] = decodeBufMap[child];
        }
        int codeBufIdx = 0;
        for (auto it: coefMap) {
            int target = it.first;
            char* codebuf; 
            if (decodeBufMap.find(target) == decodeBufMap.end()) {
                codebuf = (char*)calloc(pktSizeBytes, sizeof(char));
                decodeBufMap.insert(make_pair(target, codebuf));
            } else {
                codebuf = decodeBufMap[target];
            }
            code[codeBufIdx] = codebuf;
            targets.push_back(target);
            vector<int> curcoef = it.second;
            for (int j=0; j<col; j++) {
                matrix[codeBufIdx * col + j] = curcoef[j];
            }
            codeBufIdx++;
        }
        Computation::Multi(code, data, matrix, row, col, pktSizeBytes, "Isal", fw);
        free(matrix);
        free(data);
        free(code);
    }

    // free buffers in shortening free list
    for (auto pkt_idx : shortening_free_list) {
        free(decodeBufMap[pkt_idx]);
    }
    shortening_free_list.clear();

    decodeTime += getCurrentTime();
    decodeTime /= 1000000;

    /**
     * @brief record the disk_read_info_map
     */
    for (auto item : disk_read_pkts_map) {
        vector<int> &read_pkts = item.second;
        sort(read_pkts.begin(), read_pkts.end());
    }

    for (int node_id = 0; node_id < n; node_id++) {
        vector<int> &list = disk_read_pkts_map[node_id];

        // we first transfer items in list %w
        vector<int> offset_list;
        for (int i = 0; i < list.size(); i++) {
            offset_list.push_back(list[i]%w); 
        }
        sort(offset_list.begin(), offset_list.end()); // sort in ascending order
        reverse(offset_list.begin(), offset_list.end());
        
        // create consecutive read list
        int num_of_cons_reads = 0;
        vector<int> cons_list;
        vector<vector<int>> cons_read_list; // consecutive read list
        while (offset_list.empty() == false) { 
            int offset = offset_list.back();
            offset_list.pop_back();

            if (cons_list.empty() == true) {
                cons_list.push_back(offset);
            } else {
                // it's consecutive
                if (cons_list.back() + 1 == offset) {
                    cons_list.push_back(offset); // at to the back of prev cons_list
                } else {
                    cons_read_list.push_back(cons_list); // commits prev cons_list
                    cons_list.clear();
                    cons_list.push_back(offset); // at to the back of new cons_list
                }
            }
        }
        if (cons_list.empty() == false) {
            cons_read_list.push_back(cons_list);
        }

        printf("node id: %d, cons_read_list:\n", node_id);
        for (auto cons_list : cons_read_list) {
            for (auto offset : cons_list) {
            printf("%d ", offset);
            }
            printf("\n");
        }

        // update disk_read_info_map
        disk_read_info_map[node_id].first = cons_read_list.size();
        disk_read_info_map[node_id].second = disk_read_pkts_map[node_id].size();

        // update stats
        sum_packets_read += disk_read_pkts_map[node_id].size();
    }

    // calculate norm repair bandwidth (against RS code)
    norm_repair_bandwidth = sum_packets_read * 1.0 / (k * w);

    printf("disk read info:\n");
    for (int node_id = 0; node_id < n; node_id++) {
        printf("node_id: %d, num_disk_seeks: %d, num_pkts_read: %d, pkts: ", node_id, disk_read_info_map[node_id].first, disk_read_info_map[node_id].second);
        // printf("%d, %d\n", disk_read_info_map[node_id].first, disk_read_info_map[node_id].second);
        for (auto pkt : disk_read_pkts_map[node_id]) {
            printf("%d ", pkt);
        }
        printf("\n");
    }
    printf("packets read: %d / %d, norm repair bandwidth: %f\n", sum_packets_read, k * w, norm_repair_bandwidth);

    // // calculate straggler info
    // int straggler_node_id = -1;
    // double straggler_disk_io_time_s = 0;

    // for (int node_id = 0; node_id < n; node_id++) {
    //     double node_disk_io_time_s = 1.0 * disk_read_info_map[node_id].first * disk_seek_time_ms / 1000 + 
    //         1.0 * disk_read_info_map[node_id].second * conf->_pktSize / 1048576 / w / disk_bdwt_MBps;

    //     printf("%f\n", node_disk_io_time_s);
    //     if (node_disk_io_time_s > straggler_disk_io_time_s) {
    //         straggler_disk_io_time_s = node_disk_io_time_s;
    //         straggler_node_id = node_id;
    //     }
    // }

    // printf("straggler node_id: %d, num_seeks: %d, num_sub_pkts_read: %d, disk_io_time_s: %f\n",
        //  straggler_node_id, disk_read_info_map[straggler_node_id].first, disk_read_info_map[straggler_node_id].second, straggler_disk_io_time_s);

    // debug decode
    for (int i=0; i<failsymbols.size(); i++) {
        int failidx = failsymbols[i];
        char* curbuf  = decodeBufMap[failidx];
        cout << "failidx = " << failidx << ", value = " << (int)curbuf[0] << endl;

        int failed_node = failidx / w;

        int diff = 0;

        if (failed_node < k) {
            diff = memcmp(decodeBufMap[failidx], databuffers[failidx], pktSizeBytes * sizeof(char));
        } else {
            diff = memcmp(decodeBufMap[failidx], codebuffers[failidx - n_data_symbols], pktSizeBytes * sizeof(char));
        }
        if (diff != 0) {
            printf("error: failed to decode data of symbol %d!!!!\n", i);
        } else {
            printf("decoded data of symbol %d.\n", i);
        }
    }

    // print encode and decode time
    printf("Code: %s, data volume: %llu MiB, encode throughput: %f MiB/s, encode time: %f\n", codeName.c_str(), blockSizeBytes * k / 1048576, 1.0 * blockSizeBytes * k / 1048576 / encodeTime, encodeTime);
    printf("Code: %s, decode data volume: %llu MiB, decode throughput: %f MiB/s, decode time: %f\n", codeName.c_str(), blockSizeBytes * failed_ids.size() / 1048576, 1.0 * blockSizeBytes * failed_ids.size() / 1048576 / decodeTime, decodeTime);
}