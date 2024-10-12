#include "PMiner.h"
#include <tbb/tbb.h>
#include <mutex>
#include <ctime>
#include <sys/time.h>
#include <set>
#include <cmath>
#include <unordered_set>
#include <fstream>
#include <sstream>
using namespace std;
using namespace tbb;

PatternMatching::~PatternMatching()
{
    /* if (R_visited)
    {
        free(R_visited);
        R_visited = nullptr;
    } */

    /*if (PMR) {
        free(PMR);
        PMR = nullptr;
    }*/

    if (R_adj)
    {
        free(R_adj);
        R_adj = nullptr;
    }
    if (R_reverse_adj)
    {
        free(R_reverse_adj);
        R_reverse_adj = nullptr;
    }
    if (R_adjIndex)
    {
        free(R_adjIndex);
        R_adjIndex = nullptr;
    }
    if (R_reverseAdjIndex)
    {
        free(R_reverseAdjIndex);
        R_reverseAdjIndex = nullptr;
    }
}

void PatternMatching::init(const std::string &Output_dir, int thread_num)
{
    // Mining_result_count=0;
    // eqCircleResult.reserve(15000);
    // control_flag = 0;
    outputfile_ptr = std::make_shared<SynchronizedFile>(Output_dir); // sky211116
    ThreadNum = thread_num;
    /* R_visited = (unsigned *)malloc(vertexNum_R * sizeof(unsigned));
    memset(R_visited, 0, vertexNum_R * sizeof(unsigned)); */
}




bool PatternMatching::build_degree_Rs(const std::string &inputfile, unsigned vertexNum) 
{
    cout<<"build_degree_Rs"<<endl;
    maxID = 0;     
    edgeNum_R = 0; 
    vertexNum_R = vertexNum;
    degree_R = (Degree *)calloc(vertexNum_R, sizeof(Degree));

    ifstream infile(inputfile);
    assert(infile.is_open());
    string line;
    getline(infile, line);
    istringstream iss(line);
        
    string Nodes, Edges;
    iss>>Nodes>>vertexNum_R>>Edges>>edgeNum_R;
    vertexNum_R++; 
    cout<<"vertexNum_R: "<<vertexNum_R<<endl;
    cout<<"edgeNum_R: "<<edgeNum_R<<endl;
    unsigned from, to;

    // int equil = 0;
    while (getline(infile, line))
    {
        istringstream iss(line);
        iss >> from >> to;
        if (from != to){
          degree_R[from].outdeg++;
          degree_R[to].indeg++;
        }
        // else{
        //   equil++;
        // }
    }
    infile.close();
    // std::cout<<"edgeNum_R: "<<edgeNum_R<<std::endl;
    // cout<<"maxid: "<<maxID<<endl;
    // cout<<"vertexNum_exact: "<<vertexNum_exact.size()<<endl;
    // cout<<"equil: "<<equil<<endl;
    std::cout << "finish first read degree_Rs" << std::endl;
    return true;
}
bool PatternMatching::build_R_adjs(const std::string &inputfile) 
{
    R_adj = new unsigned[edgeNum_R];
    R_reverse_adj = new unsigned[edgeNum_R];
    R_adjIndex = new unsigned[vertexNum_R+1];
    R_reverseAdjIndex = new unsigned[vertexNum_R+1];
    unsigned *R_reverseAdjIndex_tail = new unsigned[vertexNum_R+1];
    unsigned *R_AdjIndex_tail = new unsigned[vertexNum_R+1];
    memset(R_reverseAdjIndex_tail, 0, vertexNum_R);
    memset(R_AdjIndex_tail, 0, vertexNum_R);
    R_adjIndex[0] = 0;
    R_reverseAdjIndex[0] = 0;
    R_AdjIndex_tail[0] = 0;
    R_reverseAdjIndex_tail[0] = 0;

    for (unsigned i = 1; i <=vertexNum_R; i++)
    {
        R_adjIndex[i] = R_adjIndex[i - 1] + degree_R[i - 1].outdeg;
        R_reverseAdjIndex[i] = R_reverseAdjIndex[i - 1] + degree_R[i - 1].indeg;
        R_AdjIndex_tail[i] = R_adjIndex[i];
        R_reverseAdjIndex_tail[i] = R_reverseAdjIndex[i];
    }

    ifstream infile(inputfile);
    assert(infile.is_open());
    string line;
    getline(infile, line); 
    unsigned from, to;
    while (getline(infile, line))
    {
        // if (line.empty())
        //     continue;
        istringstream iss(line);
        iss >> from >> to;
        if (from != to)
        {
            R_adj[R_AdjIndex_tail[from]++] = to;
            R_reverse_adj[R_reverseAdjIndex_tail[to]++] = from;
        }
    }
    infile.close();
    free(R_AdjIndex_tail);
    free(R_reverseAdjIndex_tail);
    std::cout << "finish first read R_adjs" << std::endl;

    
    if (degree_R)
    {
        free(degree_R);
        degree_R = nullptr;
    }

    /* for(int i=0;i<edgeNum_R;i++){
        cout<<R_adj[i]<<" ";
    }
    cout<<endl;
    for(int i=0;i<vertexNum_R;i++){
        cout<<R_adjIndex[i]<<" ";
    }
    cout<<endl; */
    // std::cout << "finish second read R_adj..." << std::endl;
    
    // bool isMul=false;
    // for(int i=0;i<vertexNum_R;i++){
    //     if(degree_R[i].outdeg<2)
    //     continue;
    //     for(int j=1;j<degree_R[i].outdeg;j++){
    //         if(R_adj[R_adjIndex[i]+j]==R_adj[R_adjIndex[i]+j-1]){
    //           cout<<i<<"--->"<<R_adj[R_adjIndex[i]+j]<<endl;
    //             cout<<"exist same edge"<<endl;
    //             isMul=true;
    //         }

    //     }
    // }
    // if(!isMul){
    //     cout<<"There are no multiple edges in the data set"<<endl;
    // }


    // int Rid = 8222;
    // unsigned rout = degree_R[Rid].outdeg;
    // unsigned rin = degree_R[Rid].indeg;

    // unsigned radjindex = R_adjIndex[Rid];
    // cout<<"degree_R[10777].outdeg: "<<rout<<endl;
    // for(int i = 0 ; i< rout; ++i){
    //   cout<<R_adj[radjindex+i]<<endl;
    // }
    // cout<<"---------------------"<<endl;
    // unsigned rradjindex = R_adjIndex[Rid];
    // cout<<"degree_R[10777].indeg: "<<rin<<endl;
    // for(int i = 0 ; i< rin; ++i){
    //   cout<<R_reverse_adj[rradjindex+i]<<endl;
    // }


    // cout<<"R_adjIndex: ";
    // for(unsigned i = 0 ; i<vertexNum_R; ++i){
    //   cout<<R_adjIndex[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_adj: ";
    // for(unsigned i = 0 ; i<edgeNum_R; ++i){
    //   cout<<R_adj[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_reverseAdjIndex: ";
    // for(unsigned i = 0 ; i<vertexNum_R; ++i){
    //   cout<<R_reverseAdjIndex[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_reverse_adj: ";
    // for(unsigned i = 0 ; i<edgeNum_R; ++i){
    //   cout<<R_reverse_adj[i]<<" ";
    // }
    // cout<<endl;
    return true;
}

void PatternMatching::FIXLINE(char *s)
{
    int len = (int)strlen(s) - 1;
    if (s[len] == '\n')
        s[len] = 0;
}
bool PatternMatching::build_degree_R(const std::string &inputfile, unsigned vertexNum) 
{
    maxID = 0;     
    edgeNum_R = 0; 
    vertexNum_R = vertexNum;
    // unordered_set<unsigned> vertexNum_exact; 
    degree_R = (Degree *)calloc(vertexNum_R, sizeof(Degree));
    // cout<<vertexNum<<"----------"<<endl;
    FILE *inf = fopen(inputfile.c_str(), "r");
    if (inf == NULL)
    {
        std::cerr << "Could not load :" << inputfile << " error: " << strerror(errno)
                  << std::endl;
    }
    assert(inf != NULL);
    // std::cout << "Reading in adjacency list format!" << std::endl;
    int maxlen = 100000000;
    char *s = (char *)malloc(maxlen); 
    size_t bytesread = 0;             
    char delims[] = " \t";            
    size_t linenum = 0;               
    size_t lastlog = 0;               
    unsigned from = 0, to = 0;        
    while (fgets(s, maxlen, inf) != NULL)
    {
        linenum++;
        FIXLINE(s);                  
        char *t = strtok(s, delims); 
        from = atoi(t);              
        maxID = max(from, maxID);  
        // vertexNum_exact.insert(from);
        unsigned num = 0;            
        while ((t = strtok(NULL, delims)) != NULL)
        {
            to = atoi(t);
            if (from != to)
            {
                // cout<<from<<"---->"<<to<<endl;
                maxID = max(to, maxID);  
                // vertexNum_exact.insert(to);
                degree_R[from].outdeg++; 
                degree_R[to].indeg++;    
                // degree_R[from].ing = true; 
                // degree_R[to].ing = true; 
                num++;
                
            }
        }
        edgeNum_R += num; 
    }
    std::cout<<"edgeNum_R: "<<edgeNum_R<<std::endl;
    cout<<"maxid: "<<maxID<<endl;
    // cout<<"vertexNum_exact: "<<vertexNum_exact.size()<<endl;
    std::cout << "finish first read degree_R" << std::endl;
    free(s);
    fclose(inf);
    return true;
}
bool PatternMatching::build_R_adj(const std::string &inputfile) 
{
    R_adj = new unsigned[edgeNum_R];
    R_reverse_adj = new unsigned[edgeNum_R];
    R_adjIndex = new unsigned[vertexNum_R+1];
    R_reverseAdjIndex = new unsigned[vertexNum_R+1];
    unsigned *R_reverseAdjIndex_tail = new unsigned[vertexNum_R+1];
    unsigned *R_AdjIndex_tail = new unsigned[vertexNum_R+1];
    memset(R_reverseAdjIndex_tail, 0, vertexNum_R);
    memset(R_AdjIndex_tail, 0, vertexNum_R);
    R_adjIndex[0] = 0;
    R_reverseAdjIndex[0] = 0;
    R_AdjIndex_tail[0] = 0;
    R_reverseAdjIndex_tail[0] = 0;
    unsigned tail = 0;
    FILE *inf = fopen(inputfile.c_str(), "r");
    if (inf == NULL)
    {
        std::cerr << "Could not load :" << inputfile << " error: " << strerror(errno)
                  << std::endl;
    }
    assert(inf != NULL);
    // std::cout << "Reading in adjacency list format!" << std::endl;
    int maxlen = 100000000;
    char *s = (char *)malloc(maxlen); 
    char delims[] = " \t";            
    size_t linenum = 0;               
    unsigned from = 0, to = 0;        
    for (unsigned i = 1; i <=vertexNum_R; i++)
    {
        R_adjIndex[i] = R_adjIndex[i - 1] + degree_R[i - 1].outdeg;
        R_reverseAdjIndex[i] = R_reverseAdjIndex[i - 1] + degree_R[i - 1].indeg;
        R_AdjIndex_tail[i] = R_adjIndex[i];
        R_reverseAdjIndex_tail[i] = R_reverseAdjIndex[i];
    }
    

    while (fgets(s, maxlen, inf) != NULL)
    {
        linenum++;
        FIXLINE(s);                  
        char *t = strtok(s, delims); 
        from = atoi(t);              
        while ((t = strtok(NULL, delims)) != NULL)
        {
            to = atoi(t);
            if (from != to)
            {
                R_adj[R_AdjIndex_tail[from]++] = to;
                R_reverse_adj[R_reverseAdjIndex_tail[to]++] = from;
            }
        }
    }
    /* for(int i=0;i<edgeNum_R;i++){
        cout<<R_adj[i]<<" ";
    }
    cout<<endl;
    for(int i=0;i<vertexNum_R;i++){
        cout<<R_adjIndex[i]<<" ";
    }
    cout<<endl; */
    // std::cout << "finish second read R_adj..." << std::endl;
    
    // bool isMul=false;
    // for(int i=0;i<vertexNum_R;i++){
    //     if(degree_R[i].outdeg<2)
    //     continue;
    //     for(int j=1;j<degree_R[i].outdeg;j++){
    //         if(R_adj[R_adjIndex[i]+j]==R_adj[R_adjIndex[i]+j-1]){
    //           cout<<i<<"--->"<<R_adj[R_adjIndex[i]+j]<<endl;
    //             cout<<"exist same edge"<<endl;
    //             isMul=true;
    //         }

    //     }
    // }
    // if(!isMul){
    //     cout<<"There are no multiple edges in the data set"<<endl;
    // }
 

    free(s);
    free(R_AdjIndex_tail);
    free(R_reverseAdjIndex_tail);
    fclose(inf);
    std::cout << "finish first read R_adj" << std::endl;

    
    if (degree_R)
    {
        free(degree_R);
        degree_R = nullptr;
    }


    // int Rid = 8222;
    // unsigned rout = degree_R[Rid].outdeg;
    // unsigned rin = degree_R[Rid].indeg;

    // unsigned radjindex = R_adjIndex[Rid];
    // cout<<"degree_R[10777].outdeg: "<<rout<<endl;
    // for(int i = 0 ; i< rout; ++i){
    //   cout<<R_adj[radjindex+i]<<endl;
    // }
    // cout<<"---------------------"<<endl;
    // unsigned rradjindex = R_adjIndex[Rid];
    // cout<<"degree_R[10777].indeg: "<<rin<<endl;
    // for(int i = 0 ; i< rin; ++i){
    //   cout<<R_reverse_adj[rradjindex+i]<<endl;
    // }


    // cout<<"R_adjIndex: ";
    // for(unsigned i = 0 ; i<vertexNum_R; ++i){
    //   cout<<R_adjIndex[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_adj: ";
    // for(unsigned i = 0 ; i<edgeNum_R; ++i){
    //   cout<<R_adj[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_reverseAdjIndex: ";
    // for(unsigned i = 0 ; i<vertexNum_R; ++i){
    //   cout<<R_reverseAdjIndex[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"R_reverse_adj: ";
    // for(unsigned i = 0 ; i<edgeNum_R; ++i){
    //   cout<<R_reverse_adj[i]<<" ";
    // }
    // cout<<endl;
    return true;
}
bool PatternMatching::build_P_adj(const std::string &inputfile, unsigned vertexNum)
{
    vertexNum_P = vertexNum;
    edgeNum_P = 0;
    degree_P = (Degree *)calloc(vertexNum_P, sizeof(Degree));
    std::vector<std::vector<P_ID>> tmp(vertexNum_P, std::vector<P_ID>(vertexNum_P)); 
    FILE *inf = fopen(inputfile.c_str(), "r");
    if (inf == NULL)
    {
        std::cerr << "Could not load :" << inputfile << " error: " << strerror(errno)
                  << std::endl;
    }
    assert(inf != NULL);
    // std::cout << "Reading in adjacency list format!" << std::endl;
    int maxlen = 100000000;
    char *s = (char *)malloc(maxlen); 
    char delims[] = " \t";            
    size_t linenum = 0;              
    unsigned from = 0, to = 0;        
    while (fgets(s, maxlen, inf) != NULL)
    {
        linenum++;
        FIXLINE(s);                  
        char *t = strtok(s, delims); 
        from = atoi(t);             
        while ((t = strtok(NULL, delims)) != NULL)
        {
            to = atoi(t);
            if (from != to)
            {
                degree_P[from].outdeg++; 
                degree_P[to].indeg++;    
                tmp[from][to] = 1;
                edgeNum_P++;
            }
        }
    }
    P_adj = tmp;
    std::cout << "finish read P_adj..."
              << "|E| = " << edgeNum_P << std::endl;
    free(s);
    fclose(inf);
    return true;
}






bool PatternMatching::matchPR_expand()
{
    
    // sel.resize(vertexNum_P); 
    // minMatchID = 0;         
    // minMatchNum = UINT_MAX;  
    // unsigned current_size = 0;
    // unsigned total_size = 0; 
    // for (unsigned i = 0; i < vertexNum_P; i++) 
    // {
    //     for (R_ID j = 0; j < vertexNum_R; j++)
    //     {
    //         int Routdeg = R_adjIndex[j+1] - R_adjIndex[j];
    //         int Rindeg = R_reverseAdjIndex[j+1] - R_reverseAdjIndex[j];
    //         if (Rindeg >= degree_P[i].indeg && Routdeg >= degree_P[i].outdeg)

    //         {
    //             current_size++;
    //         }
    //     }
    //     sel[i] = current_size;
    //     total_size += current_size;

    //     if (current_size < minMatchNum)
    //     {
    //         minMatchNum = current_size;
    //         minMatchID = i;
    //     }
    //     current_size = 0;
    // }
    // // minMatchID = 4;
    // std::cout << "minMatchID = " << minMatchID << std::endl;
    // minMatchIDs.push_back(minMatchID);

    
    int degree = 0;
    int curd = 0;
    for(int i = 0; i < vertexNum_P; ++i){
      curd = degree_P[i].indeg + degree_P[i].outdeg;
      if(curd == degree){
        minMatchIDs.push_back(i);
      }else if(curd > degree){
        minMatchIDs.clear();
        minMatchIDs.push_back(i);
        degree = curd;
      }
    }

    cout<<"minMatchIDs: ";
    for(auto& a : minMatchIDs){
      cout<<a<<" ";
    }
    cout<<endl;

    
    return true;
}

bool PatternMatching::PVAllVisited(vector<int> P_visited)
{
    for (auto k : P_visited)
    {
        if (k == 0)
            return false;
    }
    return true;
}


void PatternMatching::findSym()
{
    // cout << "-----" << endl;
    vector<int> P_visited(vertexNum_P, 0);
    vector<vector<P_ID>> P_adjcp = P_adj;
    int setNum = 1; 
    while (!PVAllVisited(P_visited))
    {
        int curID;
        for (int i = 0; i < vertexNum_P; i++)
        {
            if (P_visited[i] == 0)
            {
                curID = i;
                break;
            }
        }
        P_visited[curID] = 1;
        vector<int> nums; 
        for (int i = 0; i < vertexNum_P; i++)
        {
            if (degree_P[i].indeg == degree_P[curID].indeg && degree_P[i].outdeg == degree_P[curID].outdeg && i != curID)
            {
                nums.push_back(i);
            }
        }
        if (nums.empty())
            continue;
        
        for (auto tmpid : nums)
        {
            vector<vector<unsigned>> PMRcpy(vertexNum_P);
            vector<int> selcpy(vertexNum_P);
            long long result = 0;
            bool branchFinish = true;
            int visited_edgeNum = 0;
            int oriID = tmpid;
            unordered_map<R_ID, int> isTraversed;
            // cout<<"in tmpid "<<tmpid<<"  curid "<<curID<<endl;
            sym_searchPG(PMRcpy, selcpy, P_adjcp, tmpid, curID, branchFinish, result, oriID, isTraversed);
            // cout<<"findSym---result"<<result<<endl;
            // cout<<"out tmpid "<<tmpid<<"  curid "<<curID<<endl;
            if (result != 0)
            {
                P_visited[tmpid] = 1;
                sym[tmpid] = setNum;
                sym[curID] = setNum;
            }
        }
        setNum++;
    }
    
    for (auto it = sym.begin(); it != sym.end(); it++)
    {
        P_ID id = it->first;
        int group = it->second;
        sym_group[group].insert(id);
    }

    
    int eqNum = sym.size();          
    int eqSetNum = sym_group.size(); 
    int vecRow = vertexNum_P - eqNum + eqSetNum;
    //vector<vector<int>> eqVec;
    Equivalent_order.resize(vecRow);
    P_ID curid = 0;
    unordered_map<P_ID,int> mp;
    for (int i = 0; i < vecRow; i++)
    {
        while(mp.count(curid)!=0&&curid<vertexNum_P){
            curid++;
        }
        if (sym.count(curid) != 0)
        {
            int curSetNum = sym[curid];
            auto eqVertex = sym_group[curSetNum];
            for (auto it = eqVertex.begin(); it != eqVertex.end(); ++it)
            {
                mp[*it]=1;
                Equivalent_order[i].push_back(*it);
            }
        }
        else
        {
            mp[curid]=1;
            Equivalent_order[i].push_back(curid);
        }
    }



    
    isSym = false;
    if(sym.size() > 0){
      isSym = true;
      cout<<"sym:yes"<<endl;
      // cout<<"exist sym....."<<endl;
      // cout<<"Equivalent_order:"<<endl;
      // for(auto vec:Equivalent_order){
      //     for(auto k:vec){
      //         cout<<k<<" ";
      //     }
      //     cout<<endl;
      // }
    }else
    {
      cout<<"sym:no"<<endl;
    }
    

    
    // for (auto it = sym_group.begin(); it != sym_group.end(); it++)
    // {
    //     auto s = it->second;
    //     P_ID first = *(s.begin());
    //     P_ID second = *(++s.begin());
    //     // cout<<first<<",,,,,,,"<<second<<endl;
    //     if(P_adj[first][second] == 1 || P_adj[second][first] == 1){
    //       isEqCircle = true;
    //       circleSize = s.size();
    //     }
    // }

    // sky20231007
    
    
    if(sym_group.size() == 1 && sym_group[1].size() == vertexNum_P && edgeNum_P == vertexNum_P){
      isEqCircle = true;
      circleSize = vertexNum_P;
    }

    if (isEqCircle)
    {
        cout << "exist equivalent circle" << endl;
        cout << "circleSize: "<<circleSize << endl;

    }


    
    // if(sym_group.size() > 1){
    //   std::vector<unsigned> minMatchID_PPMR;
    //   for (R_ID j = 0; j < vertexNum_R; j++)
    //   {
    //       if (degree_P[j].indeg >= degree_P[minMatchID].indeg && degree_P[j].outdeg >= degree_P[minMatchID].outdeg)
    //       {
    //           minMatchID_PPMR.emplace_back(j);
    //           // R_visited[j] = 1;
    //       }
    //   }
    //   cout<<"=========================================================================="<<endl;
    //   cout<<"minMatchID_PPMR:"<<endl;
    //   for(int i = 0; i< minMatchID_PPMR.size();i++){
    //     cout<<minMatchID_PPMR[i]<<" ";
    //   }
    //   cout<<endl;
    //   symNum = 0;
    //   for (int i = 0; i < minMatchID_PPMR.size(); i++)
    //   {
    //         vector<vector<unsigned>> PMRcpy(vertexNum_P);
    //         vector<int> selcpy(vertexNum_P);
    //         long long result = 0;
    //         bool branchFinish = true;
    //         int visited_edgeNum = 0;
    //         int current_match_RID = minMatchID_PPMR[i];
    //         int oriID = current_match_RID;
    //         unordered_map<R_ID, int> isTraversed;
    //         isTraversed[oriID] = 1;
    //         // cout<<"in tmpid "<<tmpid<<"  curid "<<curID<<endl;
    //         sym_searchPG(PMRcpy, selcpy, P_adjcp, current_match_RID, minMatchID, branchFinish, result, oriID, isTraversed);
    //         symNum +=result;
    //   } 
    //   cout<<"symNum: "<<symNum<<endl;
    // }
    
    // cout<<"Equivalent_order:"<<endl;
    // for(auto vec:Equivalent_order){
    //     for(auto k:vec){
    //         cout<<k<<" ";
    //     }
    //     cout<<endl;
    // }
    
    // cout << "sym:" << endl;
    // for (auto it = sym.begin(); it != sym.end(); ++it)
    // {
    //     cout << it->first << " " << it->second << endl;
    // }
    
    cout << "sym_group" << endl;
    for (auto it = sym_group.begin(); it != sym_group.end(); it++){
       cout<<it->first<<": ";
       for (auto iter = it->second.begin(); iter != it->second.end(); iter++){
         cout<<*iter<<" ";
       }
       cout<<endl;
    }
    // print_P_adj(P_adj);
}

void PatternMatching::sym_searchPG(std::vector<std::vector<unsigned>> PMR_copy, std::vector<int> sel_copy, std::vector<std::vector<P_ID>> P_adj_copy, 
R_ID current_match_RID, P_ID current_match_PID, bool branchFinish, long long &result, int ori_centerID, unordered_map<R_ID, int> isTraversed)
{
    
    
    
    if (current_match_RID == ori_centerID)
    {
        for (int i = 0; i < vertexNum_P; ++i)
        {
            if (i != current_match_PID)
            {
                sel_copy[i] = INT_MAX;
            }
        }
    }
    
    /*PMR_copy[current_match_PID].resize(1);
    PMR_copy[current_match_PID][0] = current_match_RID;*/
    
    std::vector<unsigned> temp;
    temp.emplace_back(current_match_RID);
    PMR_copy[current_match_PID].swap(temp);
    sel_copy[current_match_PID] = 1;


   
    while (!isNextEPatternEmpty(P_adj_copy))
    {   
        // cout<<"---------while----start--------------"<<endl;
        // P_ID minSelId = getMaxSel_cur(current_match_PID);
        
        P_ID neighborID = UINT_MAX; 
        // int minMatchNum = INT_MAX;
        for (P_ID i = 0; i < vertexNum_P; i++)
        {
            if (P_adj_copy[i][current_match_PID] == 1)
            {
                // minMatchNum = sel[i];
                neighborID = i;

                sym_reverse_extendEdgePattern(current_match_PID, neighborID, current_match_RID, PMR_copy, sel_copy, P_adj_copy, branchFinish, isTraversed);
                if (branchFinish == false)
                {
                    return;
                }
            }
        }
        for (P_ID i = 0; i < vertexNum_P; i++)
        {
            if (P_adj_copy[current_match_PID][i] == 1)
            {
                // minMatchNum = sel[i];
                neighborID = i;
                sym_extendEdgePattern(current_match_PID, neighborID, current_match_RID, PMR_copy, sel_copy, P_adj_copy, branchFinish, isTraversed);
                if (branchFinish == false)
                {
                    return;
                }
            }
        }

        

        for (P_ID i = 0; i < vertexNum_P; ++i)
        {
            for (P_ID j = i + 1; j < vertexNum_P; ++j)
            {
                if ((P_adj_copy[i][j] == 1 || P_adj_copy[j][i] == 1) && (sel_copy[i] != INT_MAX || sel_copy[j] != INT_MAX))
                {
                    if (sel_copy[i] <= sel_copy[j])
                    {
                        current_match_PID = i;
                        neighborID = j;
                    }
                    else
                    {
                        current_match_PID = j;
                        neighborID = i;
                    }
                    
                    for (auto match_RID : PMR_copy[current_match_PID])
                    {
                        if (isTraversed.count(match_RID) == 0)
                        {
                            isTraversed[match_RID] = 1;
                            sym_searchPG(PMR_copy, sel_copy, P_adj_copy, match_RID, current_match_PID, branchFinish, result, ori_centerID, isTraversed);
                            isTraversed.erase(match_RID);
                        }
                    }
                    branchFinish = false;
                }
            }
        }
        // cout<<"---------while----end--------------"<<endl;


    }

    if (branchFinish == false)
    {
    // cout<<"---------sym_searchPG----end;--------------"<<endl;
        return;
    }
    // print_PMR(PMR_copy);
    long long cur_count = 1;
    for (unsigned i = 0; i < vertexNum_P; ++i)
    {
        cur_count *= PMR_copy[i].size();
    }
    result += cur_count;
    // cout<<"---------sym_searchPG----end--------------"<<endl;

    return;
}

void PatternMatching::sym_reverse_extendEdgePattern(P_ID v_pt, P_ID v_ps, R_ID cur_r_vt, std::vector<std::vector<unsigned>> &PMR_copy, std::vector<int> &sel_copy, 
std::vector<std::vector<P_ID>> &P_adj, bool &branchFinish, unordered_map<R_ID, int> isTraversed)
{
    // cout<<"---------sym_reverse_extendEdgePattern----start--------------"<<endl;
    // cout<<"("<<v_ps<<","<<v_pt<<")"<<"----"<<cur_r_vt<<endl;
    
    if (P_adj[v_ps][v_pt] == 0)
    {
        std::cout << "Error: can't reverse_extendEdgePattern, point " << v_pt << " and " << v_ps << " don't have reverse edge." << std::endl;
        return;
    }
    else
    {
        P_adj[v_ps][v_pt] = 2; 
    }
    
    if (branchFinish == false)
        return;
    /* if(R_visited[cur_r_vt]==1&&cur_r_vt>ori_centerID)
    return;  */
    std::vector<R_ID> Mtemp;
    for (int i = 0; i < vertexNum_P; i++)
    {
      // cout<<"isTraversed: "<<endl;
      // for(auto it = isTraversed.begin(); it != isTraversed.end(); it++){
      //   cout<<it->first<<" "<<endl;
      // }
        if (P_adj[i][cur_r_vt] != 0 && degree_P[i].indeg == degree_P[v_ps].indeg && degree_P[i].outdeg == degree_P[v_ps].outdeg && isTraversed.count(i) == 0)
        {
            Mtemp.push_back(i);
        }
    }
    
    if (PMR_copy[v_ps].size() == 0)
    {
        PMR_copy[v_ps].swap(Mtemp);
        sel_copy[v_ps] = PMR_copy[v_ps].size();
    }
    
    else
    {
        intersection(Mtemp, PMR_copy[v_ps]);
        sel_copy[v_ps] = PMR_copy[v_ps].size();
    }
    if (sel_copy[v_ps] == 0)
    {
        branchFinish = false;
    }
    // print_PMR(PMR_copy);

    // cout<<"---------sym_reverse_extendEdgePattern----end--------------"<<endl;

    return;
}

void PatternMatching::sym_extendEdgePattern(P_ID v_ps, P_ID v_pt, R_ID cur_r_vs, std::vector<std::vector<unsigned>> &PMR_copy, std::vector<int> &sel_copy, 
std::vector<std::vector<P_ID>> &P_adj, bool &branchFinish, unordered_map<R_ID, int> isTraversed)
{
    // cout<<"---------sym_extendEdgePattern----start--------------"<<endl;
    // cout<<"("<<v_ps<<","<<v_pt<<")"<<"----"<<cur_r_vs<<endl;
    
    if (P_adj[v_ps][v_pt] == 0)
    {
        std::cout << "Error: can't reverse_extendEdgePattern, point " << v_pt << " and " << v_ps << " don't have edge." << std::endl;
        return;
    }
    else
    {
        P_adj[v_ps][v_pt] = 2; 
    }
   
    if (branchFinish == false)
        return;
    std::vector<R_ID> Mtemp;
    for (int i = 0; i < vertexNum_P; i++)
    {
        if (P_adj[cur_r_vs][i] != 0 && degree_P[i].indeg == degree_P[v_pt].indeg && degree_P[i].outdeg == degree_P[v_pt].outdeg && isTraversed.count(i) == 0)
        {
            Mtemp.push_back(i);
        }
    }
    
    if (PMR_copy[v_pt].size() == 0)
    {
        PMR_copy[v_pt].swap(Mtemp);
        sel_copy[v_pt] = PMR_copy[v_pt].size();
    }
    
    else
    {
        intersection(Mtemp, PMR_copy[v_pt]);
        sel_copy[v_pt] = PMR_copy[v_pt].size();
    }
    if (sel_copy[v_pt] == 0)
    {
        branchFinish = false;
    }
    // print_PMR(PMR_copy);
    // cout<<"---------sym_extendEdgePattern----end--------------"<<endl;
    return;
}


void PatternMatching::build_constraint(){
  
  for(int i = 0 ; i< vertexNum_P; i++){
    
    std::multimap<int, P_ID>  outEdge;  
    std::multimap<int, P_ID>  inEdge;  
    for(int j = 0; j< vertexNum_P; j++){ 
      if(P_adj[i][j] == 1){ 
        outEdge.insert({degree_P[j].indeg+degree_P[j].outdeg, j});
      }
      if(P_adj[j][i] == 1){ 
        inEdge.insert({degree_P[j].indeg+degree_P[j].outdeg, j});
      }
    }
    std::vector<P_ID> outEdgeId; 
    std::vector<P_ID> inEdgeId; 
    for(auto it = outEdge.begin(); it!=outEdge.end(); it++){
      outEdgeId.push_back(it->second);
    }
    for(auto it = inEdge.begin(); it!=inEdge.end(); it++){
      inEdgeId.push_back(it->second);
    }
    extend_oreder.push_back(outEdgeId);
    extend_oreder.push_back(inEdgeId);
  }

  // cout<<"extend_oreder: "<<endl;
  // for(int i = 0; i<extend_oreder.size()/2; i++){
  //   for(int j = 0; j<extend_oreder[i*2].size(); j++){
  //     cout<<extend_oreder[i*2][j]<<" ";
  //   }
  //   cout<<endl;
  //   for(int j = 0; j<extend_oreder[i*2+1].size(); j++){
  //     cout<<extend_oreder[i*2+1][j]<<" ";
  //   }
  //   cout<<endl;
  // }
  // cout<<"extend_oreder: "<<endl;
  // for(int i = 0; i<extend_oreder.size()/2; i++){
  //   if(extend_oreder[i*2].size()> 0){
 
  //     for(int j = 0; j<extend_oreder[i*2].size(); j++){
  //       cout<<extend_oreder[i*2][j]<<" ";
  //     }
  //     cout<<endl;
  //   }
  //   if(extend_oreder[i*2+1].size()>0){
  
  //     for(int j = 0; j<extend_oreder[i*2+1].size(); j++){
  //       cout<<extend_oreder[i*2+1][j]<<" ";
  //     }
  //     cout<<endl;
  //   }
  // }

  
  std::vector<std::vector<P_ID>> temp_antecedent(vertexNum_P, std::vector<P_ID>(vertexNum_P, INT_MAX)); 
  for(int i = 0; i<extend_oreder.size()/2; i++){
    for(int j = 0; j<extend_oreder[i*2].size(); j++){ 
      P_ID target = extend_oreder[i*2][j];
      if(j == 0){ 
        temp_antecedent[i][target] = INT_MAX;
      }else{
        P_ID tempID = INT_MAX; 
        for(int k = 0; k<j; k++){ 
          P_ID ktarget = extend_oreder[i*2][k];
          if((degree_P[ktarget].indeg + degree_P[ktarget].outdeg) <= (degree_P[target].indeg + degree_P[target].outdeg)){ 
            if(tempID == INT_MAX){
              tempID = ktarget;
            }else{ 
              if((degree_P[ktarget].indeg + degree_P[ktarget].outdeg) > (degree_P[tempID].indeg + degree_P[tempID].outdeg)){
                tempID = ktarget;
              }
            }
          }
        }
        temp_antecedent[i][target] = tempID;
      }
    }
    for(int j = 0; j<extend_oreder[i*2+1].size(); j++){ 
      P_ID target = extend_oreder[i*2+1][j];
      if(j == 0){ 
        temp_antecedent[i][target] = INT_MAX;
      }else{
        P_ID tempID = INT_MAX; 
        for(int k = 0; k<j; k++){ 
          P_ID ktarget = extend_oreder[i*2+1][k];
          if((degree_P[ktarget].indeg + degree_P[ktarget].outdeg) <= (degree_P[target].indeg + degree_P[target].outdeg)){ 
            if(tempID == INT_MAX){
                tempID = ktarget;
            }else{ 
              if((degree_P[ktarget].indeg + degree_P[ktarget].outdeg) > (degree_P[tempID].indeg + degree_P[tempID].outdeg)){
                tempID = ktarget;
              }
            }
          }
        }
        temp_antecedent[i][target] = tempID;
      }
    }
  }
  antecedent_pid = temp_antecedent;

  // cout<<"antecedent_pid: "<<endl;
  // for(int i =0; i<antecedent_pid.size()+1; i++){
  //   if(i == 0){
  //     cout<<"x ";
  //   }else{
  //     cout<<i-1<<" ";
  //   }
  // }
  // cout<<endl;
  // for(int i =0; i<antecedent_pid.size(); i++){
  //   cout<<i<<" ";
  //   for(int j =0; j<antecedent_pid[i].size();j++){
  //     if(antecedent_pid[i][j] == INT_MAX){
  //       cout<<"x ";
  //     }else{
  //       cout<<antecedent_pid[i][j]<<" ";
  //     }
  //   }
  //   cout<<endl;
  // }

  
  std::vector<std::vector<std::vector<P_ID>>> temp_successor(vertexNum_P, std::vector<std::vector<P_ID>>(vertexNum_P,std::vector<P_ID>())); 
  for(int i = 0; i<vertexNum_P;i++){
    for(int j=0; j< vertexNum_P;j++){
      P_ID extendId = antecedent_pid[i][j];
      if( extendId != INT_MAX){
        temp_successor[i][extendId].push_back(j);
      }
    }
  }
  successor_pid = temp_successor;
  // cout<<"successor_pid:"<<endl;
  // for(int i = 0; i<vertexNum_P;i++){
  //   for(int j=0; j< vertexNum_P;j++){
  //     std::vector<P_ID> constraint_temp = successor_pid[i][j];
  //     if( constraint_temp.size() > 0){
  //       cout<<i<<"--->"<<j<<": ";
  //       for(int k = 0; k<constraint_temp.size(); k++ ){
  //         cout<<constraint_temp[k]<<" ";
  //       }
  //       cout<<endl;
  //     }
  //   }
  // }

  
  std::vector<std::vector<std::vector<P_ID>>> temp_str_successor(vertexNum_P, std::vector<std::vector<P_ID>>(vertexNum_P,std::vector<P_ID>())); 
  for(int i = 0; i<vertexNum_P;i++){
    for(int j=0; j< vertexNum_P;j++){
      std::vector<P_ID> constraint_temp = successor_pid[i][j];
      if( constraint_temp.size() > 0){
        std::vector<P_ID> str_temp;
        for(int k = 0; k<constraint_temp.size(); k++ ){
          if(degree_P[constraint_temp[k]].indeg == degree_P[j].indeg && degree_P[constraint_temp[k]].outdeg == degree_P[j].outdeg){
            str_temp.push_back(constraint_temp[k]);
          }
        }
        temp_str_successor[i][j].swap(str_temp);
      }
    }
  }
  str_successor_pid = temp_str_successor;
  // cout<<"str_successor_pid:"<<endl;
  // for(int i = 0; i<vertexNum_P;i++){
  //   for(int j=0; j< vertexNum_P;j++){
  //     std::vector<P_ID> constraint_temp = str_successor_pid[i][j];
  //     if( constraint_temp.size() > 0){
  //       cout<<i<<"--->"<<j<<": ";
  //       for(int k = 0; k<constraint_temp.size(); k++ ){
  //         cout<<constraint_temp[k]<<" ";
  //       }
  //       cout<<endl;
  //     }
  //   }
  // }
}

void PatternMatching::build_center_order(){
  
  int order_sel = INT32_MAX; 
  for(auto& minMatchID : minMatchIDs){
    std::vector<std::vector<P_ID>> P_adj_copy = P_adj;
    vector<P_ID> temp_center_order; 
    int center_oreder_size = 0;
    int cur_order_sel = 0;
    vector<int> marked(vertexNum_P, 0) ; 
    center_order_dfs(minMatchID, P_adj_copy, temp_center_order, center_oreder_size, cur_order_sel, order_sel, marked);
  }
  // center_order = {4,0,1,5};
  // center_order = {4,5,2,0};
  cout<<"center_order: ";
  for(int j = 0 ; j< center_order.size() ; ++j){
    cout<<center_order[j]<<" ";
  }
  cout<<endl;

  
 
  minMatchID = center_order[0];
  std::cout << "minMatchID = " << minMatchID << std::endl;

  
  need_full = vertexNum_P-center_order.size();
  cout<<"need_full: "<<need_full<<endl;
 
  unordered_set<P_ID> co_temp(center_order.begin(),center_order.end());
  vector<int> temp;
  for(int i = 0; i < vertexNum_P; ++i){
    if(co_temp.count(i) == 0){
      temp.push_back(i);
    }
  }
  full_index = temp;
  cout<<"full_index: ";
  for(auto& a : full_index){
    cout<<a<<" ";
  }
  cout<<endl;


  
 
  need_full_no_dup_rem = true;
  
}

void PatternMatching::center_order_dfs(P_ID preCenter, std::vector<std::vector<P_ID>>P_adj_copy, vector<P_ID> temp_center_order, int center_oreder_size, 
                                       int cur_order_sel, int & order_sel, vector<int> marked){
  marked[preCenter] = 2; 

  vector<P_ID> next_center; 

  
  
  temp_center_order.push_back(preCenter);
  // print_P_adj(P_adj_copy);

  
  int intersection_num = 0; 
  for(int i = 0; i< vertexNum_P; ++i){
    if(P_adj_copy[preCenter][i]!=0){ 
      if(marked[i] == 1) ++intersection_num; 
      if(P_adj_copy[preCenter][i] == 1){
        P_adj_copy[preCenter][i] = 2; 
        marked[i] = 1; 
        next_center.push_back(i); 
      }
    }
    if(P_adj_copy[i][preCenter]!=0){ 
      if(marked[i] == 1) ++intersection_num; 
      if(P_adj_copy[i][preCenter] == 1){
        P_adj_copy[i][preCenter] = 2; 
        marked[i] = 1; 
        next_center.push_back(i);
      }
    }

  }
  
  cur_order_sel += (intersection_num+1)*pow(10,center_oreder_size);
  ++center_oreder_size;
  
  if(!isNextEPatternEmpty(P_adj_copy)){
    
    if(next_center.size() > 0){ 
      
      
      for(int j = 0; j<next_center.size(); ++j){
        center_order_dfs(next_center[j], P_adj_copy, temp_center_order, center_oreder_size, cur_order_sel, order_sel, marked);
      }
    }else{
      temp_center_order.pop_back();
     
      vector<P_ID> nect_extend; 
      for(int i = 0 ; i < temp_center_order.size(); ++i){
          P_ID pid = temp_center_order[i];
          for(int j = 0; j < vertexNum_P; ++j){
            if(P_adj_copy[pid][j] == 2 && marked[j] == 1){ 
              nect_extend.push_back(j);
            }
            if(P_adj_copy[j][pid] == 2 && marked[j] == 1){ 
              nect_extend.push_back(j);
            }
          }
      }
      // cout<<"nect_extend: ";
      // for(int j = 0 ; j< nect_extend.size() ; ++j){
      //   cout<<nect_extend[j]<<" ";
      // }
      // cout<<endl;
      
      for(int j = 0; j<nect_extend.size(); ++j){
        center_order_dfs(nect_extend[j], P_adj_copy, temp_center_order, center_oreder_size, cur_order_sel, order_sel, marked);
      }

    }

  }else{
    
    if(cur_order_sel < order_sel){
      // print_P_adj(P_adj_copy);
      // cout<<"temp_center_order: ";
      // for(int j = 0 ; j< temp_center_order.size() ; ++j){
      //   cout<<temp_center_order[j]<<" ";
      // }
      // cout<<endl;
      // cout<<"cur_order_sel: "<<cur_order_sel<<endl;
      center_order = temp_center_order;  
      order_sel = cur_order_sel; 
    }
  }
  return;

}

void PatternMatching::searchALLCircle()
{
    std::cout << "Start Circle graph mining..." << std::endl;
    int degree_P_in = degree_P[minMatchID].indeg;
    int degree_P_out = degree_P[minMatchID].outdeg;
    for (R_ID j = 0; j < vertexNum_R; j++)
    {
        int Routdeg = R_adjIndex[j+1] - R_adjIndex[j];
        int Rindeg = R_reverseAdjIndex[j+1] - R_reverseAdjIndex[j];
        if (Rindeg >= degree_P_in && Routdeg >= degree_P_out)

        {
            minMatchID_PMR.emplace_back(j);
        }
    }
    unsigned minMatchID_PMR_num = minMatchID_PMR.size();
    // tbb::task_scheduler_init init(ThreadNum);
    if(circleSize == 3){
      std::cout << "----three--circle--mining----" << std::endl;
      tbb::task_scheduler_init init(ThreadNum);
      unsigned long long finalAns = parallel_reduce(blocked_range<size_t>(0, minMatchID_PMR.size()), (unsigned long long)0, [&](blocked_range<size_t> r, long long ans)
                              {    
                                  for(int i=r.begin();i!=r.end();++i){
                                      ans+=three_Circle_Multithreaded_search(i);
                                      // ans+=three_full_Circle_Multithreaded_search(i);

                                  }
                                  return ans; 
                              },plus<long long>()
                              // ,simple_partitioner{} 
                              );
      std::cout << "Mining result count is : " << finalAns << std::endl;
      // std::cout << "Mining result count is : " << finalAns/3 << std::endl;


    }else if (circleSize == 4 ){
      std::cout << "----four--circle--mining----" << std::endl;
      tbb::task_scheduler_init init(ThreadNum);
      unsigned long long finalAns = parallel_reduce(blocked_range<size_t>(0, minMatchID_PMR.size()), (unsigned long long)0, [&](blocked_range<size_t> r, long long ans)
                              {    
                                  for(int i=r.begin();i!=r.end();++i){
                                      ans+=four_Circle_Multithreaded_search(i);
                                      // ans+=four_full_Circle_Multithreaded_search(i);
                                  }
                                  return ans; 
                              },plus<long long>())
                              // ,simple_partitioner{} 
                              ;
      // std::cout << "Mining result count is : " << finalAns << std::endl;
      std::cout << "Mining result count is : " << finalAns << std::endl;
    }else{
      unsigned long long finalAns = parallel_reduce(blocked_range<size_t>(0, minMatchID_PMR.size()), 0, [&](blocked_range<size_t> r, long long ans)
                              {    
                                  for(int i=r.begin();i!=r.end();++i){
                                      ans+=eqCircle_Multithreaded_search(i);
                                  }
                                  return ans; 
                              },plus<long long>()
                              ,simple_partitioner{} 
                              );
      std::cout << "Mining result count is : " << finalAns << std::endl;
    }

    
    // unsigned long long finalAns = 0;
    // for (int i = 0; i < minMatchID_PMR.size(); i++)
    // {
    //     finalAns += Multithreaded_search(i);
    // } 
    // std::cout << "Mining result count is : " << finalAns << std::endl;
}


unsigned long long PatternMatching::three_Circle_Multithreaded_search(unsigned i){
  unsigned long long result = 0;
  R_ID first_R_id = minMatchID_PMR[i];

  unsigned first_out_start = R_adjIndex[first_R_id];
  unsigned first_out_end = R_adjIndex[first_R_id+1];

  for (unsigned i = first_out_start; i < first_out_end ; ++i)
  {
    R_ID second_R_id = R_adj[i];
    // result += 1;
    if(second_R_id > first_R_id){
      result += R_adj_merge(first_R_id, second_R_id);
    }
    // result += R_adj_merge(first_R_id, second_R_id);
  }

  return result;
}


unsigned long long PatternMatching::three_full_Circle_Multithreaded_search(unsigned i){
  unsigned long long result = 0;
  R_ID first_R_id = minMatchID_PMR[i];

  unsigned first_out_start = R_adjIndex[first_R_id];
  unsigned first_out_end = R_adjIndex[first_R_id+1];

  for (unsigned i = first_out_start; i < first_out_end ; ++i)
  {
    R_ID second_R_id = R_adj[i];
    if(second_R_id > first_R_id){
      vector<R_ID> PMR ;
      R_adj_merge_full(first_R_id, second_R_id, PMR);
      result += PMR.size();
      for(int j = 0; j < PMR.size(); ++j){
        cout<<first_R_id<<" "<<second_R_id<<" "<<PMR[j]<<endl;
      }
    }
  }
  return result;
}


unsigned long long PatternMatching::four_Circle_Multithreaded_search(unsigned i){
  unsigned long long result = 0;
  R_ID first_R_id = minMatchID_PMR[i];
  unsigned first_out_start = R_adjIndex[first_R_id];
  unsigned first_out_end = R_adjIndex[first_R_id+1];
  for (unsigned i = first_out_start; i < first_out_end ; ++i)
  {
    R_ID second_R_id = R_adj[i];
    if(second_R_id > first_R_id){
      unsigned second_out_start = R_adjIndex[second_R_id];
      unsigned second_out_end = R_adjIndex[second_R_id+1];
      for (unsigned j = second_out_start; j < second_out_end ; ++j){
        R_ID three_R_id = R_adj[j];
        // result += 1;
        if(three_R_id> first_R_id){
          result += R_adj_merge_four(first_R_id, second_R_id, three_R_id);
        }
      }
    }
  }
  return result;
}


unsigned long long PatternMatching::four_full_Circle_Multithreaded_search(unsigned i){
  unsigned long long result = 0;
  R_ID first_R_id = minMatchID_PMR[i];
  unsigned first_out_start = R_adjIndex[first_R_id];
  unsigned first_out_end = R_adjIndex[first_R_id+1];
  for (unsigned i = first_out_start; i < first_out_end ; ++i)
  {
    R_ID second_R_id = R_adj[i];
    if(second_R_id > first_R_id){
      unsigned second_out_start = R_adjIndex[second_R_id];
      unsigned second_out_end = R_adjIndex[second_R_id+1];
      for (unsigned j = second_out_start; j < second_out_end ; ++j){
        R_ID three_R_id = R_adj[j];
        // result += 1;
        if(three_R_id> first_R_id){
          vector<R_ID> PMR ;
          R_adj_merge_full_four(first_R_id,second_R_id, three_R_id, PMR);
          result += PMR.size();
          for(int j = 0; j < PMR.size(); ++j){
            cout<<first_R_id<<" "<<second_R_id<<" "<<three_R_id<<" "<<PMR[j]<<endl;
          }
        }
      }
    }
  }
  return result;
}


unsigned PatternMatching::R_adj_merge(R_ID first_R_id, R_ID last_R_id){
  // cout<<"R_adj_merge: "<<first_R_id<<"----"<<last_R_id<<endl;
    unsigned count = 0;

    unsigned istart = R_reverseAdjIndex[first_R_id];
    unsigned iend = R_reverseAdjIndex[first_R_id+1];
    unsigned jstart = R_adjIndex[last_R_id];
    unsigned jend = R_adjIndex[last_R_id+1];

    while(istart < iend && jstart < jend)
    {
        if(R_reverse_adj[ istart ] == R_adj[jstart]) 
        {  
            if(R_adj[jstart] > first_R_id) {
              count += 1;
            }
            // count += 1;
            istart += 1;
            jstart += 1;
        }
        else if(R_reverse_adj[ istart ] < R_adj[jstart])   {
            istart += 1;
        }
        else
        {
            jstart += 1;
        }
    }

    return count;
}


void PatternMatching::R_adj_merge_full(R_ID first_R_id, R_ID last_R_id, vector<R_ID> &PMR){
  // cout<<"R_adj_merge: "<<first_R_id<<"----"<<last_R_id<<endl;
    unsigned istart = R_reverseAdjIndex[first_R_id];
    unsigned iend = R_reverseAdjIndex[first_R_id+1];
    unsigned jstart = R_adjIndex[last_R_id];
    unsigned jend = R_adjIndex[last_R_id+1];
    while(istart < iend && jstart < jend)
    {   
        if(R_reverse_adj[ istart ] == R_adj[jstart]) 
        {   
            R_ID tempId = R_adj[jstart];
            if(tempId > first_R_id){
              PMR.push_back(tempId);
            }
            istart += 1;
            jstart += 1;
        }
        else if(R_reverse_adj[ istart ] < R_adj[jstart])   
        {
            istart += 1;
        }
        else
        {
            jstart += 1;
        }
    }
}



unsigned PatternMatching::R_adj_merge_four(R_ID first_R_id, R_ID second_R_id,  R_ID last_R_id){
  // cout<<"R_adj_merge: "<<first_R_id<<"----"<<last_R_id<<endl;
    unsigned count = 0;

    unsigned istart = R_reverseAdjIndex[first_R_id];
    unsigned iend = R_reverseAdjIndex[first_R_id+1];
    unsigned jstart = R_adjIndex[last_R_id];
    unsigned jend = R_adjIndex[last_R_id+1];
    while(istart < iend && jstart < jend)
    {
        if(R_reverse_adj[ istart ] == R_adj[jstart]) 
        {  
            unsigned four_R_id = R_adj[jstart];
            if(four_R_id > first_R_id && four_R_id != second_R_id ) {
              count += 1;
            }
            // count += 1;
            istart += 1;
            jstart += 1;
        }
        else if(R_reverse_adj[ istart ] < R_adj[jstart])   
        {
            istart += 1;
        }
        else
        {
            jstart += 1;
        }
    }

    return count;
}


void PatternMatching::R_adj_merge_full_four(R_ID first_R_id, R_ID second_R_id, R_ID last_R_id, vector<R_ID> &PMR){
  // cout<<"R_adj_merge: "<<first_R_id<<"----"<<last_R_id<<endl;
    unsigned istart = R_reverseAdjIndex[first_R_id];
    unsigned iend = R_reverseAdjIndex[first_R_id+1];
    unsigned jstart = R_adjIndex[last_R_id];
    unsigned jend = R_adjIndex[last_R_id+1];
    while(istart < iend && jstart < jend)
    {   
        if(R_reverse_adj[ istart ] == R_adj[jstart]) 
        {   
            R_ID tempId = R_adj[jstart];
            if(tempId != second_R_id && tempId > first_R_id){
              PMR.push_back(tempId);
            }
            istart += 1;
            jstart += 1;
        }
        else if(R_reverse_adj[ istart ] < R_adj[jstart])   
        {
            istart += 1;
        }
        else
        {
            jstart += 1;
        }
    }
}


unsigned long long PatternMatching::eqCircle_Multithreaded_search(R_ID i)
{
    /* std::vector<std::vector<unsigned>> PMR_initialValue;
    PMR_initialValue.resize(vertexNum_P);
    PMR_initialValue[minMatchID].emplace_back(minMatchID_PMR[i]);
    vector<int> sel_cp = sel;
    bool branchFinish = true;
    int visited_edgeNum = 0;
    int ori_centerID = minMatchID_PMR[i]; 
    vector<vector<P_ID>> P_adjcp = P_adj;
    int result = 0;
    eqCircle_searchPG(PMR_initialValue, sel_cp, P_adjcp, minMatchID_PMR[i], minMatchID, branchFinish, ori_centerID, 0, result);
    return result; */
    // cout<<"oir "<<ori_centerID<<endl;
    vector<R_ID> PMR_copy;
    PMR_copy.resize(vertexNum_P);
    PMR_copy[0] = minMatchID_PMR[i];
    long long result = 0;
    unordered_map<R_ID, int> mp;
    mp[PMR_copy[0]] = 1;
    minDFS(PMR_copy, minMatchID_PMR[i], result, 0, mp);
    /*  int r_result = 0;
     maxDFS(PMR_copy, minMatchID_PMR[i], r_result, 0,mp); */
    return result;
}

void PatternMatching::minDFS(vector<R_ID> PMR_copy, R_ID ori_centerId, long long &result, int curTime, unordered_map<R_ID, int> mp)
{
    if (curTime == circleSize - 1)
    {
        R_ID rid = PMR_copy[vertexNum_P - 1];
        unsigned rid_out_start = R_adjIndex[rid];
        unsigned rid_in_end = R_adjIndex[rid+1];
        for (unsigned i = rid_out_start; i < rid_in_end; i++)
        {
            R_ID tmpid = R_adj[i];
            if (tmpid == PMR_copy[0])
            {
                result++;
                return;
            }
        }
        return;
    }
    R_ID Rcurid = PMR_copy[vertexNum_P - 1];
    unsigned Rcurid_out_start = R_adjIndex[Rcurid];
    unsigned Rcurid_in_end = R_adjIndex[Rcurid+1];
    for (unsigned i = Rcurid_out_start; i < Rcurid_in_end; i++)
    {
        R_ID tmpid = R_adj[i];
        if (tmpid > ori_centerId && mp.count(tmpid) == 0)
        {
            PMR_copy[curTime + 1] = tmpid;
            mp[tmpid] = 1;
            minDFS(PMR_copy, ori_centerId, result, curTime + 1, mp);
            mp.erase(tmpid);
        }
    }
}






void PatternMatching::searchAllPR()
{
    std::cout << "Start graph mining..." << std::endl;
    // Mining_result_count = 0; 
    // cout << "P_adj:" << endl;
    // for (P_ID i = 0; i < vertexNum_P; i++)
    // {
    //     for (P_ID j = 0; j < vertexNum_P; j++)
    //     {
    //         cout <<P_adj[i][j];
    //     }
    // }
    // cout<<endl;
    // std::vector<int> minMatchID_PMR;
    // minMatchID = 1;
    // cout<<"minMatchID: "<<minMatchID<<endl;
    // struct timeval start_time, end_time, tmp_start, tmp_end;
    // gettimeofday(&start_time, NULL);
    int degree_P_in = degree_P[minMatchID].indeg;
    int degree_P_out = degree_P[minMatchID].outdeg;
    // int count = 0;
    for (R_ID j = 0; j < vertexNum_R; j++)
    {
        int Routdeg = R_adjIndex[j+1] - R_adjIndex[j];
        int Rindeg = R_reverseAdjIndex[j+1] - R_reverseAdjIndex[j];
        if (Rindeg >= degree_P_in && Routdeg >= degree_P_out)
        {
            minMatchID_PMR.emplace_back(j);
            // count++;
        }
        // if(count <= 443172){
        //     minMatchID_PMR.emplace_back(j);
        //     count++;
        // }
    }
    // gettimeofday(&tmp_end, NULL);
    // std::cout << "minMatchID_PMR:" << ((tmp_end.tv_sec - start_time.tv_sec) * 1000000 + (tmp_end.tv_usec - start_time.tv_usec)) / 1000000.0 << " s" << std::endl;
    // std::cout << "minMatchID = " << minMatchID << std::endl;
    // cout<<"minMatchID_PMR:"<<endl;
    // for(int i = 0; i< minMatchID_PMR.size();i++){
    //   cout<<minMatchID_PMR[i]<<" ";
    // }
    // cout<<endl;
    
    
    
    unsigned minMatchID_PMR_num = minMatchID_PMR.size();
    // cout<<"minMatchID_PMR.size(): "<<minMatchID_PMR_num<<endl; 
    // print_totxt_start(minMatchID_PMR);
    tbb::task_scheduler_init init(ThreadNum);
    unsigned long long finalAns = parallel_reduce(blocked_range<size_t>(0, minMatchID_PMR_num), (unsigned long long)0, [&](blocked_range<size_t> r, long long ans)
                                        {        
                                            for(int i=r.begin();i!=r.end();++i){
                                                // unsigned long long tmp = Multithreaded_search(i);
                                                // cout<<"tmp: "<<tmp<<endl;
                                                // ans+=tmp;
                                                ans+=Multithreaded_search(i);
                                            }
                                            return ans;
                                        },plus<long long>()
                                        ,simple_partitioner{} 
                                        );
   
    // unsigned long long finalAns = 0;
    // for (int i = 0; i < minMatchID_PMR.size(); i++)
    // {
    //     finalAns += Multithreaded_search(i);
    // } 
    std::cout << "Mining result count is : " << finalAns << std::endl;
}


unsigned long long PatternMatching::Multithreaded_search(R_ID i)
{
    // cout << "P_adj:" << endl;
    // for (P_ID i = 0; i < vertexNum_P; i++)
    // {
    //     for (P_ID j = 0; j < vertexNum_P; j++)
    //     {
    //         cout <<P_adj[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }
    // cout<<endl;
    std::vector<std::vector<unsigned>> PMR;
    PMR.resize(vertexNum_P);
    std::vector<std::vector<P_ID>> P_adj_copy = P_adj;
    int P_center_index = 0; 
    unordered_set<R_ID> isTraversed; 
    isTraversed.insert(minMatchID_PMR[i]); 
    PMR[center_order[P_center_index]].emplace_back(minMatchID_PMR[i]);
    unsigned long long Mining_result_count = searchPG(PMR, P_adj_copy, minMatchID_PMR[i], P_center_index,isTraversed);
    return Mining_result_count;
}

unsigned long long PatternMatching::searchPG(std::vector<std::vector<unsigned>> PMR_copy, std::vector<std::vector<P_ID>> P_adj_copy, R_ID current_match_RID,
int P_center_index, unordered_set<R_ID>& isTraversed)
{
    P_ID current_match_PID = center_order[P_center_index];
    P_center_index++;
    // cout<<"---------searchPG----start--------------"<<endl;
    
    
    // std::vector<unsigned> temp;
    // temp.emplace_back(current_match_RID);
    // PMR_copy[current_match_PID].swap(temp);

    
    
    if(!extendEdgePattern(current_match_PID, current_match_RID, PMR_copy, P_adj_copy,isTraversed)) return (unsigned long long)0; 
    
    if(!reverse_extendEdgePattern(current_match_PID, current_match_RID, PMR_copy, P_adj_copy,isTraversed)) return (unsigned long long)0;
    
    if(P_center_index < center_order.size()){ 
      unsigned long long result = 0;
      current_match_PID = center_order[P_center_index];
      
      std::vector<unsigned> temp; // sky 20230508
      PMR_copy[current_match_PID].swap(temp);// sky 20230508
      for (auto match_RID : temp)
      {
          if (isTraversed.count(match_RID) == 0)
          {
              isTraversed.insert(match_RID);
              PMR_copy[current_match_PID].emplace_back(match_RID); // sky 20230508
              result += searchPG(PMR_copy, P_adj_copy, match_RID, P_center_index, isTraversed);
              PMR_copy[current_match_PID].pop_back(); // sky 20230508
              isTraversed.erase(match_RID);
          }
      }
      return result;
    }else{
      
      // return count_set(PMR_copy,isTraversed); 
      return count_full(PMR_copy,isTraversed); 
      // return out_full(); 
      // return 1;

    }
    // cout<<"---------searchPG----end--------------"<<endl;
}


bool PatternMatching::isNextEPatternEmpty(std::vector<std::vector<P_ID>> &P_adj_copy)
{
    for (unsigned i = 0; i < vertexNum_P; i++)
    {
        for (unsigned j = i + 1; j < vertexNum_P; j++)
        {
            if (P_adj_copy[i][j] == 1 || P_adj_copy[j][i] == 1)
                return false;
        }
    }
    return true;
}


bool PatternMatching::extendEdgePattern(P_ID v_ps, R_ID cur_r_vs, std::vector<std::vector<unsigned>> &PMR_copy,
std::vector<std::vector<P_ID>> &P_adj, unordered_set<R_ID>& isTraversed)
{
    // cout<<"---------extendEdgePattern----start--------------"<<endl;
    // cout<<"("<<v_ps<<")"<<"----"<<cur_r_vs<<endl;
   
    vector<P_ID> order_min; 
    vector<P_ID> order_int; 
    int order_size = extend_oreder[v_ps*2].size();
    for(int i = 0 ; i< order_size ;i++){
      P_ID neighborID = extend_oreder[v_ps*2][i];
      if(P_adj[v_ps][neighborID] == 1  ){ 
        P_adj[v_ps][neighborID] = 2; 
        if(PMR_copy[neighborID].size() ==0){
          order_min.emplace_back(neighborID);
        }else{
          order_int.emplace_back(neighborID);
        }
      }
    }

    // cout<<"order_min: ";
    // for(int i = 0 ;i< order_min.size(); i++){
    //   cout<<order_min[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"order_int: ";
    // for(int i = 0 ;i< order_int.size(); i++){
    //   cout<<order_int[i]<<" ";
    // }
    // cout<<endl;

    


    unsigned R_out_start = R_adjIndex[cur_r_vs];
    unsigned R_out_end = R_adjIndex[cur_r_vs+1];
    int order_min_size = order_min.size();
    if(order_min_size > 0 ){
      for (unsigned i = R_out_start; i < R_out_end ; ++i)
      {
        R_ID tmpid = R_adj[i];
        int tmp_degree_R_in = R_reverseAdjIndex[tmpid+1] - R_reverseAdjIndex[tmpid];
        int tmp_degree_R_out = R_adjIndex[tmpid+1] - R_adjIndex[tmpid];
        if(isTraversed.count(tmpid) == 0){
          for(int j = 0 ; j< order_min_size; j++){
            P_ID tmp_p =  order_min[j];
            if ( tmp_degree_R_in >= degree_P[tmp_p].indeg && tmp_degree_R_out >= degree_P[tmp_p].outdeg)
            {
                
                PMR_copy[tmp_p].emplace_back(tmpid);
            }
          }
        }
      }
      for(int j = 0; j<order_min_size; j++){
        P_ID tmp_p =  order_min[j];
        if (PMR_copy[tmp_p].size() == 0)
        {
            return false; 
        }
      }
    }


    int order_int_size = order_int.size() ;
    if(order_int_size > 0){
      for(int j = 0; j<order_int_size; j++){
        P_ID tmp_p =  order_int[j];

        
        std::vector<R_ID> intersect_result;
        std::vector<R_ID> cur_PMR(PMR_copy[tmp_p]);
        unsigned istart = R_out_start;
        int jstart = 0;
        int iend = R_out_end;
        int jend = cur_PMR.size();
        while(istart < iend && jstart < jend)
        {
            if(R_adj[ istart ] == cur_PMR[jstart]) 
            {   R_ID tmpid = cur_PMR[jstart];
                if(isTraversed.count(tmpid) == 0){
                  intersect_result.emplace_back(tmpid);
                }
                istart += 1;
                jstart += 1;
            }
            else if(R_adj[ istart ] < cur_PMR[jstart])   
            {
                istart += 1;
            }
            else
            {
                jstart += 1;
            }
        }


        PMR_copy[tmp_p].swap(intersect_result);
        if (PMR_copy[tmp_p].size() == 0)
        {
            return false;  
        }
      }
    }

    // print_PMR(PMR_copy);
    // cout<<"---------extendEdgePattern----end--------------"<<endl;

    return true; 
}


bool PatternMatching::reverse_extendEdgePattern(P_ID v_pt, R_ID cur_r_vt, std::vector<std::vector<unsigned>> &PMR_copy,
std::vector<std::vector<P_ID>> &P_adj, unordered_set<R_ID>& isTraversed)
{
    // cout<<"---------reverse_extendEdgePattern----start--------------"<<endl;
    // cout<<"("<<v_pt<<")"<<"----"<<cur_r_vt<<endl;
    

    vector<P_ID> order_min; 
    vector<P_ID> order_int; 
    int order_size = extend_oreder[v_pt*2+1].size();
    for(int i = 0 ; i< order_size ;i++){
      P_ID neighborID = extend_oreder[v_pt*2+1][i];
      if(P_adj[neighborID][v_pt] == 1  ){ 
        P_adj[neighborID][v_pt] = 2; 
        if(PMR_copy[neighborID].size() == 0){
          order_min.emplace_back(neighborID);
        }else{
          order_int.emplace_back(neighborID);
        }
      }
    }

    // cout<<"order_min: ";
    // for(int i = 0 ;i< order_min.size(); i++){
    //   cout<<order_min[i]<<" ";
    // }
    // cout<<endl;
    // cout<<"order_int: ";
    // for(int i = 0 ;i< order_int.size(); i++){
    //   cout<<order_int[i]<<" ";
    // }
    // cout<<endl;

    unsigned R_out_start = R_reverseAdjIndex[cur_r_vt];
    unsigned R_out_end = R_reverseAdjIndex[cur_r_vt+1];
    int order_min_size = order_min.size();
    if(order_min_size > 0 ){
      for (unsigned i = R_out_start; i < R_out_end ; ++i)
      {
        R_ID tmpid = R_reverse_adj[i];
        int tmp_degree_R_in = R_reverseAdjIndex[tmpid+1] - R_reverseAdjIndex[tmpid];
        int tmp_degree_R_out = R_adjIndex[tmpid+1] - R_adjIndex[tmpid];
        if(isTraversed.count(tmpid) == 0){
          for(int j = 0 ; j< order_min_size; j++){
            P_ID tmp_p =  order_min[j];
            if ( tmp_degree_R_in >= degree_P[tmp_p].indeg && tmp_degree_R_out >= degree_P[tmp_p].outdeg)
            {
               
                PMR_copy[tmp_p].emplace_back(tmpid);
            }
          }
        }
      }
      for(int j = 0; j<order_min_size; j++){
        P_ID tmp_p =  order_min[j];
        if (PMR_copy[tmp_p].size() == 0)
        {
            return false;
        }
      }
    }

    int order_int_size = order_int.size() ;
    if(order_int_size > 0){
      for(int j = 0; j<order_int_size; j++){
        P_ID tmp_p =  order_int[j];
        
        
        // std::unordered_set<R_ID> temp(PMR_copy[tmp_p].begin(), PMR_copy[tmp_p].end());
        // // PMR_copy[tmp_p].clear();
        // std::vector<R_ID> intersect_result;
        // for (int i = 0; i < degree_R_in ; ++i)
        // {
        //   R_ID tmpid = R_reverse_adj[ cur_R_reverseAdjIndex + i];
        //   auto p = temp.find(tmpid);
        //   if (p != temp.end())
        //   {
        //       // std::cout << i << " ";
        //       if(isTraversed.count(tmpid) == 0){
        //         temp.erase(tmpid);
        //         intersect_result.emplace_back(tmpid);
        //       }
        //   }
        // }

        
        std::vector<R_ID> intersect_result;
        std::vector<R_ID> cur_PMR(PMR_copy[tmp_p]);
        unsigned istart = R_out_start;
        int jstart = 0;
        unsigned iend = R_out_end;
        int jend = cur_PMR.size();
        while(istart < iend && jstart < jend)
        {
            if(R_reverse_adj[ istart ] == cur_PMR[jstart]) 
            {   R_ID tmpid = cur_PMR[jstart];
                if(isTraversed.count(tmpid) == 0){
                  intersect_result.emplace_back(tmpid);
                }
                istart += 1;
                jstart += 1;
            }
            else if(R_reverse_adj[ istart ] < cur_PMR[jstart])   {
                istart += 1;
            }
            else
            {
                jstart += 1;
            }
        }

        PMR_copy[tmp_p].swap(intersect_result);
        if (PMR_copy[tmp_p].size() == 0)
        {
            return false;
        }
      }
    }
    // print_PMR(PMR_copy);
    // cout<<"---------reverse_extendEdgePattern----end--------------"<<endl;
    return true;
}


unsigned long long PatternMatching::count_set(std::vector<std::vector<unsigned>> &PMR_copy, unordered_set<R_ID>& isTraversed){
  if(need_full == 0){
    return 1;
  }else{
    
    vector<vector<unsigned>> PMR_remain;  
    PMR_remain.resize(need_full);

    for(int i = 0 ; i< need_full; ++i){
      PMR_remain[i].swap(PMR_copy[full_index[i]]);
    }

    
   
    vector<unsigned> isTraversed_v(isTraversed.begin(), isTraversed.end());
    sort(isTraversed_v.begin(), isTraversed_v.end());
    for(int i = 0 ; i<need_full; ++i){
      eliminate(PMR_remain[i], isTraversed_v);
    }


    if(need_full == 1){
      return PMR_remain[0].size();
    }else if(need_full == 2){
      return set_operation2(PMR_remain);
    }else if (need_full == 3){
      return set_operation3(PMR_remain);
    }else if ( need_full == 4 ){
      return set_operation4(PMR_remain);
    }else if ( need_full == 5 ){
      unsigned long long result = 0;
      
      int min_size = PMR_remain[0].size();
      int min_index = 0;
      for(int i = 1; i<need_full; ++i){
        int PMR_remain_size = PMR_remain[i].size();
        if(PMR_remain_size < min_size){
          min_size = PMR_remain_size;
          min_index = i;
        }
      }

     
      unordered_set<unsigned> union_set; 
      vector<unsigned> full_v(PMR_remain[min_index]);
      PMR_remain.erase(PMR_remain.begin()+min_index);
      for(int i = 0; i<4;++i){
        merge_un_set(full_v, PMR_remain[i], union_set);
      }
      // cout<<"union_set: ";
      // for(auto it = union_set.begin(); it != union_set.end(); ++it){
      //   cout<<*it<<" ";
      // }
      // cout<<endl;
      // cout<<"gggggg"<<endl;

      
      unsigned four_line_count = 0;
      if(union_set.size() < full_v.size()){
        four_line_count = set_operation4(PMR_remain);
      }
      // cout<<"hhhhhhh"<<endl;
      
      
      int full_v_size = full_v.size();
      for(int i = 0 ; i<full_v_size;++i){

        unsigned rid = full_v[i];
        // cout<<"rid: "<<rid<<endl;
        if(union_set.count(rid) != 0){
          vector<vector<unsigned>> PMR_remain_copy(PMR_remain);
          vector<unsigned> isTraversed_cur_v;
          isTraversed_cur_v.push_back(rid);
          // cout<<"lllllll"<<endl;
          // print_PMR(PMR_remain_copy);
          for(int j = 0; j<4;++j){
            eliminate(PMR_remain_copy[j], isTraversed_cur_v);
          }
          // cout<<"nnnnnn"<<endl;

          result += set_operation4(PMR_remain_copy);
        }else{
          result += four_line_count;
        }
      }
      return result;
    }else{
      cout<<"con not use set operation optimization..."<<endl;
    }
  }
}


unsigned long long PatternMatching::count_full(std::vector<std::vector<unsigned>> &PMR_copy, unordered_set<R_ID>& isTraversed){
  // cout<<"====================count_full=============="<<endl;
  if(need_full == 0){
    return 1;
  }else{
    
    vector<vector<unsigned>> PMR_remain;  
    PMR_remain.resize(need_full);

    
    multimap<int, int> mp; 
    for(int i = 0; i < need_full; ++i){
      mp.insert({PMR_copy[full_index[i]].size(), full_index[i]});
    }
    int tindex = 0;
    for(auto& t : mp){
      PMR_remain[tindex].swap(PMR_copy[t.second]);
      ++tindex;
    }

   
   
    vector<unsigned> isTraversed_v(isTraversed.begin(), isTraversed.end());
    sort(isTraversed_v.begin(), isTraversed_v.end());
    for(int i = 0 ; i<need_full; ++i){
      eliminate(PMR_remain[i], isTraversed_v);
    }


    if(need_full == 1){
      return PMR_remain[0].size();
    }else if(need_full == 2){
      return full_permutation2(PMR_remain);
    }else if (need_full == 3){
      return full_permutation3(PMR_remain);
    }else if ( need_full == 4 ){
      return full_permutation4(PMR_remain);
    }else if ( need_full == 5 ){
      return full_permutation5(PMR_remain);
    }else{
      unordered_set<unsigned> set;
      return full_permutation(PMR_remain, 0, set);
    }
  }
}


bool PatternMatching::eliminate(std::vector<unsigned> &PMR_remain, std::vector<unsigned> &isTraversed){
  // cout<<"PMR_remain: ";
  // for(int i = 0 ; i< PMR_remain.size();++i){
  //   cout<<PMR_remain[i]<<" ";
  // }
  // cout<<endl;
  // cout<<"isTraversed: "<<isTraversed[0]<<endl;
  bool flag = false;
  int i,j;
  
  i = j = 0;
  while(i < PMR_remain.size() && j < isTraversed.size())
  {
      if(PMR_remain[i] == isTraversed[j]) 
      {
          PMR_remain.erase(PMR_remain.begin() + i);
          flag = true; 
      }
      else if(PMR_remain[i] < isTraversed[j])   {
          i += 1;
      }
      else
      {
          j += 1;
      }
  }
  return flag;
}

unsigned PatternMatching::set_operation2(std::vector<std::vector<unsigned>> &PMR_copy){
  unsigned cur_count = 0;
  cur_count = PMR_copy[0].size() * PMR_copy[1].size() - merge_count(PMR_copy[0], PMR_copy[1]);
  return cur_count;
}

unsigned PatternMatching::set_operation3(std::vector<std::vector<unsigned>> &PMR_copy){
  unsigned cur_count = 0;
  std::vector<unsigned> merge;
  int line1 = PMR_copy[0].size();
  int line2 = PMR_copy[1].size();
  int line3 = PMR_copy[2].size();
  if(need_full_no_dup_rem){
    
    return line1 * line2 * line3;
  }
  merge_set(PMR_copy[0], PMR_copy[1], merge);
  unsigned count12 = merge.size()*line3; 
  unsigned count13 = merge_count(PMR_copy[0], PMR_copy[2])*line2; 
  unsigned count23 = merge_count(PMR_copy[1], PMR_copy[2])*line1; 
  unsigned count123 = merge_count(merge, PMR_copy[2])*2; 
  cur_count = line1*line2*line3 - count12 - count13 - count23 + count123;
  return cur_count;
}

unsigned PatternMatching::set_operation4(std::vector<std::vector<unsigned>> &PMR_copy){
  unsigned cur_count = 0;
  int line1 = PMR_copy[0].size();
  int line2 = PMR_copy[1].size();
  int line3 = PMR_copy[2].size();
  int line4 = PMR_copy[3].size();
  std::vector<unsigned> merge12;
  std::vector<unsigned> merge34;
  std::vector<unsigned> merge123;

  merge_set(PMR_copy[0], PMR_copy[1], merge12); 
  merge_set(PMR_copy[2], PMR_copy[3], merge34); 
  merge_set(merge12, PMR_copy[2], merge123); 

  unsigned size12 = merge12.size(); 
  unsigned size13 = merge_count(PMR_copy[0], PMR_copy[2]); 
  unsigned size14 = merge_count(PMR_copy[0], PMR_copy[3]); 
  unsigned size23 = merge_count(PMR_copy[1], PMR_copy[2]); 
  unsigned size24 = merge_count(PMR_copy[1], PMR_copy[3]); 
  unsigned size34 = merge34.size(); 
  unsigned size123 = merge123.size(); 
  unsigned size124 = merge_count(merge12, PMR_copy[3]); 
  unsigned size234 = merge_count(merge34, PMR_copy[1]); 
  unsigned size134 = merge_count(merge34, PMR_copy[0]); 
  unsigned size1234 = merge_count(merge123, PMR_copy[3]); 

  unsigned count12 = size12*line3*line4 - size12*size34; 
  unsigned count13 = size13*line2*line4 - size13*size24; 
  unsigned count14 = size14*line2*line3 - size14*size23; 
  unsigned count23 = size23*line1*line4; 
  unsigned count24 = size24*line1*line3; 
  unsigned count34 = size34*line1*line2; 
  unsigned count123 = size123*2*line4; 
  unsigned count124 = size124*2*line3; 
  unsigned count234 = size234*2*line1; 
  unsigned count134 = size134*2*line2; 
  unsigned count1234 = size1234*2 - size1234*2*4; 


  cur_count = line1*line2*line3*line4 - count12 - count13 - count14 - count23 - count24 - count34 + count123 + count124 + count234 + count134 + count1234 ;
  return cur_count;
}

unsigned long long PatternMatching::full_permutation2(std::vector<std::vector<unsigned>> &PMR_remain){
  // cout<<3<<endl;
  unsigned long long cur_count = 0;
  int line1 = PMR_remain[0].size();
  int line2 = PMR_remain[1].size();
  for(int i1 = 0; i1 < line1; ++i1){
    int id1 = PMR_remain[0][i1];
    for(int i2 = 0; i2 < line2; ++i2){
      if(id1 != PMR_remain[1][i2]) ++cur_count;
    }
  }
  return cur_count;
}

unsigned long long PatternMatching::full_permutation3(std::vector<std::vector<unsigned>> &PMR_remain){
  // cout<<3<<endl;
  unsigned long long cur_count = 0;
  int line1 = PMR_remain[0].size();
  int line2 = PMR_remain[1].size();
  int line3 = PMR_remain[2].size();
  if(need_full_no_dup_rem){
    
    for(int i1 = 0; i1 < line1; ++i1){
      // int id1 = PMR_remain[0][i1];
      for(int i2 = 0; i2 < line2; ++i2){
        // int id2 = PMR_remain[1][i2];
        // if(id1 != id2){
          for(int i3 = 0; i3 < line3; ++i3){
            // int id3 = PMR_remain[2][i3];
            // if(id3!=id1 && id3!=id2)  ++cur_count;
             ++cur_count;
          }
        // }
      }
    }

  }else{
    
    for(int i1 = 0; i1 < line1; ++i1){
      int id1 = PMR_remain[0][i1];
      for(int i2 = 0; i2 < line2; ++i2){
        int id2 = PMR_remain[1][i2];
        if(id1 != id2){
          for(int i3 = 0; i3 < line3; ++i3){
            int id3 = PMR_remain[2][i3];
            if(id3!=id1 && id3!=id2)  ++cur_count;
          }
        }
      }
    }
  }
  return cur_count;
}

unsigned long long PatternMatching::full_permutation4(std::vector<std::vector<unsigned>> &PMR_remain){
  // cout<<4<<endl;
  unsigned long long cur_count = 0;
  int line1 = PMR_remain[0].size();
  int line2 = PMR_remain[1].size();
  int line3 = PMR_remain[2].size();
  int line4 = PMR_remain[3].size();
  for(int i1 = 0; i1 < line1; ++i1){
    int id1 = PMR_remain[0][i1];
    for(int i2 = 0; i2 < line2; ++i2){
      int id2 = PMR_remain[1][i2];
      if(id1 != id2){
        for(int i3 = 0; i3 < line3; ++i3){
          int id3 = PMR_remain[2][i3];
          if(id3!=id1 && id3!=id2){
            for(int i4 = 0; i4 < line4; ++i4){
              int id4 = PMR_remain[3][i4];
              if(id4!=id3 && id4!= id2 && id4!= id1)   ++cur_count;
            }
          }
        }
      }
    }
  }
  return cur_count;
}

unsigned long long PatternMatching::full_permutation5(std::vector<std::vector<unsigned>> &PMR_remain){
  // cout<<5<<endl;
  unsigned long long cur_count = 0;
  int line1 = PMR_remain[0].size();
  int line2 = PMR_remain[1].size();
  int line3 = PMR_remain[2].size();
  int line4 = PMR_remain[3].size();
  int line5 = PMR_remain[4].size();

  for(int i1 = 0; i1 < line1; ++i1){
    int id1 = PMR_remain[0][i1];
    for(int i2 = 0; i2 < line2; ++i2){
      int id2 = PMR_remain[1][i2];
      if(id1 != id2){
        for(int i3 = 0; i3 < line3; ++i3){
          int id3 = PMR_remain[2][i3];
          if(id3!=id1 && id3!=id2){
            for(int i4 = 0; i4 < line4; ++i4){
              int id4 = PMR_remain[3][i4];
              if(id4!=id1 && id4!=id3 && id4!= id2 && id4!= id1){
                for(int i5 = 0; i5 < line5; ++i5){
                  int id5 = PMR_remain[4][i5];
                  if(id5!=id4 && id5!=id3 && id5!= id2 && id5!= id1) ++cur_count;
                }
              }
            }
          }
        }
      }
    }
  }
  return cur_count;
}

unsigned long long PatternMatching::full_permutation(std::vector<std::vector<unsigned>> &PMR_remain, int index, unordered_set<unsigned>& set){
  if(index == PMR_remain.size()) return 1;
  unsigned long long cur_count = 0;
  int line = PMR_remain[index].size();
  for(int i = 0; i < line; ++i){
    int id = PMR_remain[index][i];
    if(set.count(id) == 0){
      set.insert(id);
      cur_count += full_permutation(PMR_remain, index+1, set);
      set.erase(id);
    }
  }
  return cur_count;
}


void PatternMatching::merge_un_set(std::vector<P_ID>& v1, std::vector<P_ID>& v2, std::unordered_set<P_ID>& un_set){
  int i,j;
  
  i = j = 0;
  while(i < v1.size() && j < v2.size())
  {
      if(v1[i] == v2[j])
      {
          un_set.insert(v1[i]);
          i += 1;
          j += 1;
      }
      else if(v1[i] < v2[j])  
      {
          i += 1;
      }
      else
      {
          j += 1;
      }
  }
}

void PatternMatching::merge_set(std::vector<P_ID>& v1, std::vector<P_ID>& v2, std::vector<P_ID>& v3){
  int i,j;
  
  i = j = 0;
  while(i < v1.size() && j < v2.size())
  {
      if(v1[i] == v2[j]) 
      {
          v3.push_back(v1[i]);
          i += 1;
          j += 1;
      }
      else if(v1[i] < v2[j])   
      {
          i += 1;
      }
      else
      {
          j += 1;
      }
  }
}

unsigned PatternMatching::merge_count(std::vector<P_ID>& v1, std::vector<P_ID>& v2){
  unsigned cur_count = 0;
  int i,j;
  
  i = j = 0;
  while(i < v1.size() && j < v2.size())
  {
      if(v1[i] == v2[j]) 
      {
          i += 1;
          j += 1;
          cur_count += 1; 
      }
      else if(v1[i] < v2[j])  
      {
          i += 1;
      }
      else
      {
          j += 1;
      }
  }

  return cur_count;
}

bool PatternMatching::intersection(std::vector<R_ID> &Mtemp, std::vector<R_ID> &PMR_copy_oneline)
{
    std::unordered_set<R_ID> temp(Mtemp.begin(), Mtemp.end());
    bool is_empty = true;

    // std::cout << "Intersection set is: ";

    std::vector<R_ID> intersect_result;
    for (auto i : PMR_copy_oneline)
    {
        auto p = temp.find(i);
        if (p != temp.end())
        {
            // std::cout << i << " ";
            temp.erase(i);
            intersect_result.emplace_back(i);
            is_empty = false;
        }
    }
    /*if (is_empty == true) {
        std::cout << "NULL";
    }*/
    // std::cout << std::endl;

    
    PMR_copy_oneline.swap(intersect_result);

    return is_empty;
}





bool PatternMatching::De_duplication(std::vector<unsigned> unchecked_res)
{
    
    unordered_set<unsigned> res_set;
    for (auto i : unchecked_res)
    {
        res_set.insert(i);
    }
    if (res_set.size() != vertexNum_P)
    {
        return false;
    }
    
    // unsigned lines = Equivalent_order.size();
    // for (unsigned i = 0; i < lines; ++i)
    // {
    //     unsigned weight = Equivalent_order[i].size();
    //     if (weight < 2)
    //     {
    //         continue;
    //     }
    //     else
    //     {
    //         unsigned pre = unchecked_res[Equivalent_order[i][0]];
    //         for (unsigned j = 1; j < weight; ++j)
    //         {
    //             unsigned cur = unchecked_res[Equivalent_order[i][j]];
    //             if (cur <= pre)
    //             {
    //                 return false;
    //             }
    //         }
    //     }
    // }
    return true;
}
unsigned PatternMatching::full_arrangement(std::vector<vector<R_ID>> cur_thread_PMR) {
  // cout<<"full_arrangement"<<endl;
  // cout<<"33333333"<<endl;
  // print_PMR(cur_thread_PMR);
    std::vector<unsigned> carry_table; 
    unsigned count = 0; 
    carry_table.assign(vertexNum_P, 0);
    for (unsigned line = 0; line < vertexNum_P; ++line)
    {
        carry_table[line] = cur_thread_PMR[line].size();
    }
    
    std::vector<unsigned> full_arrangement;
    full_arrangement.assign(vertexNum_P, 0);
    std::vector<unsigned> PMR_cur(vertexNum_P, INT_MAX);
    for (unsigned i = 0; i < vertexNum_P; ++i) {
        PMR_cur[i] = cur_thread_PMR[i][full_arrangement[i]];
    }         
     
      if (De_duplication(PMR_cur))
      {
          count++;
          // cout<<count<<endl;
      } 
    
    
    int cur_row = vertexNum_P - 1;
    while (true)
    {
        if (full_arrangement[cur_row] + 1 < carry_table[cur_row])
        {
            full_arrangement[cur_row]++;
            cur_row = vertexNum_P - 1;
           
            
            // for (auto unit : full_arrangement)
            // {
            //     std::cout << unit << ' ';
            // }
            // std::cout << std::endl;
            
            std::vector<unsigned> PMR_cur(vertexNum_P, INT_MAX);
            for (unsigned i = 0; i < vertexNum_P; ++i) {
                PMR_cur[i] = cur_thread_PMR[i][full_arrangement[i]];
            }         
              
              if (De_duplication(PMR_cur))
              {
                  count++;
                  // cout<<count<<endl;
              }
        }
        else
        {
            cur_row--;
            if (cur_row < 0)
            {
                break;
            }
            else
            {
                full_arrangement[cur_row + 1] = 0;
            }
        }
    }

    return count;
}
bool PatternMatching::check_result(std::vector<unsigned> PMR_copy)
{
    // cout<<"----------"<<endl;
    for (auto k : PMR_copy)
    {
        if (k == -1)
        {
            return false;
        }
    }
    for (int i = 0; i < vertexNum_P; i++)
    {
        for (int j = 0; j < vertexNum_P; j++)
        {
            if (P_adj[i][j] == 1)
            {
                auto R_i = PMR_copy[i];
                auto R_j = PMR_copy[j];
                int i_start = 0, i_end = 0;
                bool find_v = false;
                get_Radj_Index(R_i, i_start, i_end);
                for (int cur = i_start; cur < i_end; ++cur)
                {
                    if (R_adj[cur] == R_j)
                    {
                        // cout<<"P_i:"<<i<<" "<<"P_j:"<<j<<"   "<<"R_i:"<<R_i<<" "<<"R_j:"<<R_j<<endl;
                        find_v = true;
                        break;
                    }
                }
                if (!find_v)
                {
                    // std::cout << "An incorrect result was found: R_vertex" << R_i << " -> R_vertex" << R_j << " don't have an edge." << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}
bool PatternMatching::check_result(std::vector<std::vector<unsigned>> &PMR_copy)
{
   
    for (auto P_i : PMR_copy)
    {
        if (P_i.size() == 0)
        {
            std::cout << "There is an empty mapping set." << std::endl;
            for (int i = 0; i < vertexNum_P; ++i)
            {
                std::cout << 'P' << i << ": ";
                for (auto j : PMR_copy[i])
                {
                    std::cout << j << ", ";
                }
                std::cout << std::endl;
            }
            return false;
        }
    }
    
    for (P_ID i = 0; i < vertexNum_P; ++i)
    {
        for (P_ID j = 0; j < vertexNum_P; ++j)
        {
            if (P_adj[i][j] == 1)
            {
               
                for (auto R_i : PMR_copy[i])
                {
                    for (auto R_j : PMR_copy[j])
                    {
                        
                        int i_start = 0, i_end = 0;
                        bool find_v = false;
                        get_Radj_Index(R_i, i_start, i_end);
                        for (int cur = i_start; cur < i_end; ++cur)
                        {
                            if (R_adj[cur] == R_j)
                            {
                                find_v = true;
                                break;
                            }
                        }
                        if (!find_v)
                        {
                            std::cout << "An incorrect result was found: R_vertex" << R_i << " -> R_vertex" << R_j << " don't have an edge." << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
   
    // std::cout << "Current branch check is complete." << std::endl;
}
bool PatternMatching::get_Radj_Index(R_ID v_r, int &start, int &end)
{
    if (v_r == vertexNum_R - 1)
    {
        start = R_adjIndex[v_r];
        end = edgeNum_R;
    }
    else
    {
        start = R_adjIndex[v_r];
        end = R_adjIndex[v_r + 1];
    }
    return true;
}



void PatternMatching::print_P_adj(std::vector<std::vector<P_ID>> &P_adj)
{
    std::cout << "Print P_adj." << std::endl;
    cout<<"x ";
    for (P_ID i = 0; i < vertexNum_P; i++){
      cout<<i<<" ";
    }
    cout<<endl;
    for (P_ID i = 0; i < vertexNum_P; i++)
    {
        cout<<i<<" ";
        for (P_ID j = 0; j < vertexNum_P; j++)
        {
            cout <<P_adj[i][j]<<" ";
        }
        cout<<endl;
    }
    std::cout << "Finish print P_adj" << std::endl;
}


void PatternMatching::print_PMR(std::vector<std::vector<unsigned>> &PMR_copy)
{
    std::cout << "Print current PMR collection." << std::endl;
    for (int i = 0; i < PMR_copy.size(); i++)
    {
        std::cout << "P" << i << ": ";
        for (auto j : PMR_copy[i])
        {
              std::cout << j << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Finish print PMR collection" << std::endl;
}



void PatternMatching::print_totxt_start(vector<R_ID> minMatchID_PMR){
  sort(minMatchID_PMR.begin(),minMatchID_PMR.end());
  string outputfilename = startsIdsfilename;
  ofstream file(outputfilename);
  for(int i = 0; i < minMatchID_PMR.size(); ++i){
    file<<minMatchID_PMR[i]<<endl;
  }
  file.close();
}


void PatternMatching::input_start(vector<R_ID>& minMatchID_PMR){
  ifstream file5(startsIdsfilename);
  vector<R_ID> a;
  string line5;
  assert(file5.is_open());
  int id5;
  while (getline(file5, line5))
  {
    stringstream ss(line5);
    ss>>id5;
    a.push_back(id5);
  }    
  file5.close();
  minMatchID_PMR = a;
}
