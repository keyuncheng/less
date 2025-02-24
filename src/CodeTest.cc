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
#include <random>

#define RANDOM_SEED 12345

void generateRandomInts(mt19937 &generator, char *buffer, int size, int from, int to)
{
    uniform_int_distribution<int> distribution(from, to);
    for (int i = 0; i < size; i++)
    {
        buffer[i] = distribution(generator);
    }
}

using namespace std;

void usage()
{
    printf("usage: ./CodeTest code_name n k w blockSizeBytes <failed_block_ids>\n");
    printf("code_name: RSCONV, HHXORPlus, HTEC, BUTTERFLY, Clay, ETRSConv, ETHTEC, ETHHXORPlus, LESS\n");
    printf("n: number of blocks\n");
    printf("k: number of data blocks\n");
    printf("w: sub-packetization\n");
    printf("blockSizeBytes: size of each block in bytes\n");
    printf("failed_block_ids: list of failed block ids\n");
    printf("Example: ./CodeTest RSCONV 10 6 4 1024 1 2 3\n");
}

double getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1e+6 + (double)tv.tv_usec;
}

int main(int argc, char **argv)
{
    if (argc < 7)
    {
        usage();
        return 0;
    }

    string codeName = argv[1];
    int n = atoi(argv[2]);
    int k = atoi(argv[3]);
    int w = atoi(argv[4]);

    // block size: round up to the nearest multiple of 16 * w (for Jerasure)
    unsigned long long blockSizeBytes = atoi(argv[5]);
    if (blockSizeBytes % (16 * w) != 0)
    {
        int nearestBS = (int)ceil(1.0 * blockSizeBytes / (16 * w)) * 16 * w;
        printf("Warning: block size must be a multiple of 16 * w = %d; rounded to the nearest: %d\n", 16 * w, nearestBS);
        blockSizeBytes = nearestBS;
    }

    vector<int> failedIds;
    for (int i = 6; i < argc; i++)
    {
        int failedId = atoi(argv[i]);
        if (failedId >= n)
        {
            printf("error: invalid failed block: %d\n", failedId);
            return -1;
        }
        failedIds.push_back(failedId);
    }

    unsigned long long pktSizeBytes = blockSizeBytes / w;

    // field width (special setting for LESS)
    int fw = 8; // default fw
    if (codeName == "LESS")
    {
        uint32_t e, f;
        if (LESS::getAvailPrimElements(n, k, w, fw, e, f) == false)
        {
            printf("LESS::getAvailPrimElements() failed to find primitive element\n");
            exit(1);
        }
    }

    // double disk_seek_time_ms = stod(argv[5]);
    // double disk_bdwt_MBps = stod(argv[6]);

    // get ecId in OpenEC
    string confpath = "./conf/sysSetting.xml";
    Config *conf = new Config(confpath);

    string ecId = codeName + "_" + to_string(n) + "_" + to_string(k) + "_" + to_string(w);
    ECPolicy *ecPolicy = conf->_ecPolicyMap[ecId];
    ECBase *ec = ecPolicy->createECClass();

    /**
     * @brief Encoding
     *
     */

    // generate random data
    random_device rd;
    // mt19937 rdGenerator(RANDOM_SEED);
    mt19937 rdGenerator(rd());

    int numDataSymbols = k * w;
    int numCodeSymbols = (n - k) * w;

    // prepare buffers for data and parity blocks
    char **dataPtrs = new char *[numDataSymbols];
    char **codePtrs = new char *[numCodeSymbols];

    for (int i = 0; i < numDataSymbols; i++)
    {
        dataPtrs[i] = new char[pktSizeBytes];
        generateRandomInts(rdGenerator, dataPtrs[i], pktSizeBytes, CHAR_MIN, CHAR_MAX);
    }

    for (int i = 0; i < numCodeSymbols; i++)
    {
        codePtrs[i] = new char[pktSizeBytes];
        memset(codePtrs[i], 0, pktSizeBytes);
    }

    /**
     * @brief Encoding
     */
    // start encoding
    double encodeTime = getCurrentTime();

    // init encoding
    ECDAG *encDAG = ec->Encode();
    vector<ECTask *> encTasks;
    vector<int> encTopoSeq = encDAG->toposort();
    for (int i = 0; i < encTopoSeq.size(); i++)
    {
        ECNode *curNode = encDAG->getNode(encTopoSeq[i]);
        curNode->parseForClient(encTasks);
    }

    // encoding buffers
    unordered_map<int, char *> encBufMap;
    for (int i = 0; i < numDataSymbols; i++)
    {
        encBufMap.insert(make_pair(i, dataPtrs[i]));
    }

    for (int i = 0; i < numCodeSymbols; i++)
    {
        encBufMap.insert(make_pair(numDataSymbols + i, codePtrs[i]));
    }

    // free list to support shortening
    vector<int> shorteningFreeList;
    for (int taskId = 0; taskId < encTasks.size(); taskId++)
    {
        ECTask *computeTask = encTasks[taskId];
        // computeTask->dump();

        vector<int> children = computeTask->getChildren();
        unordered_map<int, vector<int>> coefMap = computeTask->getCoefMap();
        int col = children.size();
        int row = coefMap.size();

        // assign data and code buffers for encoding
        int *matrix = new int[row * col];
        char **data = new char *[col];
        char **code = new char *[row];
        for (int bufIdx = 0; bufIdx < children.size(); bufIdx++)
        {
            int child = children[bufIdx];

            // create buffers to support shortening
            if (child >= n * w && encBufMap.find(child) == encBufMap.end())
            {
                shorteningFreeList.push_back(child);
                char *tmpBuf = new char[pktSizeBytes];
                encBufMap[child] = tmpBuf;
            }

            data[bufIdx] = encBufMap[child];
        }
        int codeBufIdx = 0;
        for (auto it : coefMap)
        {
            int codeId = it.first;
            char *codeBuf;
            if (encBufMap.find(codeId) == encBufMap.end())
            {
                codeBuf = new char[pktSizeBytes];
                encBufMap.insert(make_pair(codeId, codeBuf));
            }
            else
            {
                codeBuf = encBufMap[codeId];
            }
            code[codeBufIdx] = codeBuf;
            vector<int> codeCoef = it.second;
            for (int j = 0; j < col; j++)
            {
                matrix[codeBufIdx * col + j] = codeCoef[j];
            }
            codeBufIdx++;
        }
        Computation::Multi(code, data, matrix, row, col, pktSizeBytes, "Isal", fw);

        delete[] matrix;
        delete[] data;
        delete[] code;
    }

    // free buffers in shortening free list
    for (auto pktIdx : shorteningFreeList)
    {
        delete[] encBufMap[pktIdx];
    }
    shorteningFreeList.clear();

    // end encoding
    encodeTime = (getCurrentTime() - encodeTime) / 1000000;

    // check coded data
    printf("Check coded data:\n");
    printf("========================\n");
    for (int i = 0; i < numDataSymbols; i++)
    {
        char *curbuf = (char *)dataPtrs[i];
        printf("dataIdx = %d, value = %d\n", i, curbuf[0]);
    }
    for (int i = 0; i < numCodeSymbols; i++)
    {
        char *curbuf = (char *)codePtrs[i];
        printf("codeIdx = %d, value = %d\n", numDataSymbols + i, curbuf[0]);
    }
    printf("========================\n");

    /**
     * @brief Decoding
     */

    vector<int> failedSymbols;
    unordered_map<int, char *> decodeBuf;

    printf("Failed blocks: ");
    for (auto failedId : failedIds)
    {
        printf("%d ", failedId);
        for (int i = 0; i < w; i++)
        {
            failedSymbols.push_back(failedId * w + i);
        }
    }
    printf("\n");

    for (int i = 0; i < failedSymbols.size(); i++)
    {
        char *tmpBuf = new char[pktSizeBytes];
        decodeBuf[failedSymbols[i]] = tmpBuf;
    }

    vector<int> availSymbols;
    for (int i = 0; i < n * w; i++)
    {
        if (find(failedSymbols.begin(), failedSymbols.end(), i) == failedSymbols.end())
        {
            availSymbols.push_back(i);
        }
    }

    // start decoding
    double decodeTime = getCurrentTime();

    ECDAG *decDAG = ec->Decode(availSymbols, failedSymbols);
    vector<ECTask *> decodeTasks;
    unordered_map<int, char *> decodeBufMap;
    vector<int> decTopoSeq = decDAG->toposort();
    for (int i = 0; i < decTopoSeq.size(); i++)
    {
        ECNode *curNode = decDAG->getNode(decTopoSeq[i]);
        curNode->parseForClient(decodeTasks);
    }
    for (int i = 0; i < numDataSymbols; i++)
    {
        if (find(failedSymbols.begin(), failedSymbols.end(), i) == failedSymbols.end())
        {
            decodeBufMap.insert(make_pair(i, dataPtrs[i]));
        }
        else
        {
            decodeBufMap.insert(make_pair(i, decodeBuf[i]));
        }
    }
    for (int i = 0; i < numCodeSymbols; i++)
        if (find(failedSymbols.begin(), failedSymbols.end(), numDataSymbols + i) == failedSymbols.end())
        {
            decodeBufMap.insert(make_pair(numDataSymbols + i, codePtrs[i]));
        }
        else
        {
            decodeBufMap.insert(make_pair(i, decodeBuf[numDataSymbols + i]));
        }

    /**
     * @brief record number of disk seeks and number of sub-packets to read for every node
     * @diskReadSymbolsMap <node> sub-packets read in each node
     * @diskReadInfoMap <nodeId, <number of disk seeks, number of sub-blocks read>>
     *
     */
    map<int, vector<int>> diskReadSymbolsMap;
    map<int, pair<int, int>> diskReadInfoMap;

    int sumPktsRead = 0;
    double normRepairBW = 0;

    // init the map
    for (int nodeId = 0; nodeId < n; nodeId++)
    {
        diskReadSymbolsMap[nodeId].clear();
        diskReadInfoMap[nodeId] = make_pair(0, 0);
    }

    for (int taskId = 0; taskId < decodeTasks.size(); taskId++)
    {
        ECTask *computeTask = decodeTasks[taskId];
        // computeTask->dump();

        vector<int> children = computeTask->getChildren();
        unordered_map<int, vector<int>> coefMap = computeTask->getCoefMap();
        int col = children.size();
        int row = coefMap.size();

        // assign data and code buffers for encoding
        int *matrix = new int[row * col];
        char **data = new char *[col];
        char **code = new char *[row];
        for (int bufIdx = 0; bufIdx < children.size(); bufIdx++)
        {
            int child = children[bufIdx];

            // create buffers to support shortening
            if (child >= n * w && decodeBufMap.find(child) == decodeBufMap.end())
            {
                shorteningFreeList.push_back(child);
                char *tmpBuf = new char[pktSizeBytes];
                decodeBufMap[child] = tmpBuf;
            }

            if (child < n * w)
            {
                int nodeId = child / w;
                vector<int> &readPkts = diskReadSymbolsMap[nodeId];
                if (find(readPkts.begin(), readPkts.end(), child) == readPkts.end())
                {
                    readPkts.push_back(child);
                }
            }

            data[bufIdx] = decodeBufMap[child];
        }
        int codeBufIdx = 0;
        for (auto it : coefMap)
        {
            int codeId = it.first;
            char *codeBuf;
            if (decodeBufMap.find(codeId) == decodeBufMap.end())
            {
                codeBuf = new char[pktSizeBytes];
                decodeBufMap.insert(make_pair(codeId, codeBuf));
            }
            else
            {
                codeBuf = decodeBufMap[codeId];
            }
            code[codeBufIdx] = codeBuf;
            vector<int> codeCoef = it.second;
            for (int j = 0; j < col; j++)
            {
                matrix[codeBufIdx * col + j] = codeCoef[j];
            }
            codeBufIdx++;
        }
        Computation::Multi(code, data, matrix, row, col, pktSizeBytes, "Isal", fw);
        delete[] matrix;
        delete[] data;
        delete[] code;
    }

    // free buffers in shortening free list
    for (auto pktIdx : shorteningFreeList)
    {
        delete[] decodeBufMap[pktIdx];
    }
    shorteningFreeList.clear();

    decodeTime = (getCurrentTime() - decodeTime) / 1000000;

    // debug decoding
    for (int i = 0; i < failedSymbols.size(); i++)
    {
        int failidx = failedSymbols[i];
        char *curbuf = decodeBufMap[failidx];

        printf("failedIdx = %d, decoded value = %d\n", failidx, (char)curbuf[0]);

        int failed_node = failidx / w;

        int diff = 0;

        if (failed_node < k)
        {
            diff = memcmp(decodeBufMap[failidx], dataPtrs[failidx], pktSizeBytes * sizeof(char));
        }
        else
        {
            diff = memcmp(decodeBufMap[failidx], codePtrs[failidx - numDataSymbols], pktSizeBytes * sizeof(char));
        }
        if (diff != 0)
        {
            printf("error: failed to decode data for symbol %d!!!!\n", i);
        }
    }

    /**
     * @brief record the diskReadInfoMap
     */
    for (auto item : diskReadSymbolsMap)
    {
        vector<int> &readPkts = item.second;
        sort(readPkts.begin(), readPkts.end());
    }

    for (int nodeId = 0; nodeId < n; nodeId++)
    {
        vector<int> &list = diskReadSymbolsMap[nodeId];

        // we first transfer items in list %w
        vector<int> offsetList;
        for (int i = 0; i < list.size(); i++)
        {
            offsetList.push_back(list[i] % w);
        }
        sort(offsetList.begin(), offsetList.end()); // sort in ascending order
        reverse(offsetList.begin(), offsetList.end());

        // create consecutive read list
        int numOfConsReads = 0;
        vector<int> consList;
        vector<vector<int>> consReadList; // consecutive read list
        while (offsetList.empty() == false)
        {
            int offset = offsetList.back();
            offsetList.pop_back();

            if (consList.empty() == true)
            {
                consList.push_back(offset);
            }
            else
            {
                // it's consecutive
                if (consList.back() + 1 == offset)
                {
                    consList.push_back(offset); // at to the back of prev consList
                }
                else
                {
                    consReadList.push_back(consList); // commits prev consList
                    consList.clear();
                    consList.push_back(offset); // at to the back of new consList
                }
            }
        }
        if (consList.empty() == false)
        {
            consReadList.push_back(consList);
        }

        printf("node id: %d, contiguous reads (of sub-blocks): ", nodeId);

        bool printGap = false;
        for (auto consList : consReadList)
        {
            for (auto offset : consList)
            {
                printf("%d ", offset);
            }
            if (printGap == false)
            {
                printGap = true;
            }
            else
            {
                printf(" || ");
            }
        }
        printf("\n");

        // update diskReadInfoMap
        diskReadInfoMap[nodeId].first = consReadList.size();
        diskReadInfoMap[nodeId].second = diskReadSymbolsMap[nodeId].size();

        // update stats
        sumPktsRead += diskReadSymbolsMap[nodeId].size();
    }

    // calculate norm repair bandwidth (against RS code)
    normRepairBW = sumPktsRead * 1.0 / (k * w);

    printf("disk read info:\n");
    for (int nodeId = 0; nodeId < n; nodeId++)
    {
        printf("nodeId: %d, number of disk seeks: %d, number of sub-blocks read: %d, sub-blocks: ", nodeId, diskReadInfoMap[nodeId].first, diskReadInfoMap[nodeId].second);
        // printf("%d, %d\n", diskReadInfoMap[nodeId].first, diskReadInfoMap[nodeId].second);
        for (auto pkt : diskReadSymbolsMap[nodeId])
        {
            printf("%d ", pkt);
        }
        printf("\n");
    }

    // // calculate straggler info
    // int straggler_nodeId = -1;
    // double straggler_disk_io_time_s = 0;

    // for (int nodeId = 0; nodeId < n; nodeId++) {
    //     double node_disk_io_time_s = 1.0 * diskReadInfoMap[nodeId].first * disk_seek_time_ms / 1000 +
    //         1.0 * diskReadInfoMap[nodeId].second * conf->_pktSize / 1048576 / w / disk_bdwt_MBps;

    //     printf("%f\n", node_disk_io_time_s);
    //     if (node_disk_io_time_s > straggler_disk_io_time_s) {
    //         straggler_disk_io_time_s = node_disk_io_time_s;
    //         straggler_nodeId = nodeId;
    //     }
    // }

    // printf("straggler nodeId: %d, num_seeks: %d, num_sub_pkts_read: %d, disk_io_time_s: %f\n",
    //  straggler_nodeId, diskReadInfoMap[straggler_nodeId].first, diskReadInfoMap[straggler_nodeId].second, straggler_disk_io_time_s);

    delete conf;

    printf("Total sub-blocks read: %d / %d, normalized repair bandwidth: %f\n", sumPktsRead, k * w, normRepairBW);

    // print encode and decode time
    printf("Code: %s, encode throughput: %f MiB/s (%llu MiB in %f seconds)\n", codeName.c_str(), 1.0 * blockSizeBytes * k / 1048576 / encodeTime, blockSizeBytes * k / 1048576, encodeTime);
    printf("Code: %s, decode throughput: %f MiB/s (%llu MiB in %f seconds)\n", codeName.c_str(), 1.0 * blockSizeBytes * k / 1048576 / decodeTime, blockSizeBytes * k / 1048576, decodeTime);
}