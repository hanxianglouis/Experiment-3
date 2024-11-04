#include<iostream>
#include<string>
#include<cstring>
#include<sstream>
#include<fstream>
#include<unistd.h>
#include<array>

using namespace std;

void file_to_matrix(array<array<string,50>,30> &a){//查看csv文件发现一共30家企业，50个专利
    fstream file;
    file.open("FirmPatent.csv",ios::in);
    string line;
    getline(file, line); // 略过标题行
    while(!file.eof() && getline(file,line)){
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '"') {
                line.erase(i, 1); // 删除引号，需要依赖逗号拆出公司代码
                i--; // 调整索引以处理连续的引号
            }
        }

        int patentid, firmid[3];
        string title;
        stringstream ss(line);

        string temp;
        getline(ss, temp, ',');
        patentid = stoi(temp);

        getline(ss, title, ',');

        getline(ss, temp, ',');
        firmid[0] = stoi(temp);
        temp.clear();
        getline(ss, temp, ',');
        if(!temp.empty()){//有些专利只有一个公司有，后面的firmid为空
            firmid[1] = stoi(temp);
        }else{
            firmid[1] = 0;
        }
    
        temp.clear();
        getline(ss, temp, ',');
        if(!temp.empty()){
            firmid[2]= stoi(temp);
        }else{
            firmid[2]= 0;
        }
        for(int i=0;i<3 && firmid[i]!=0;i++){
            a[firmid[i]-1001][patentid-100001]=title;
        }
    }
    file.close();
    return;
}

struct triple{
    int row;
    int col;
    string title;

    triple() : row(-1), col(-1), title("") {}//默认构造函数
};
//KMP算法
int *get_next(char *t){
    int len=strlen(t);
    int *next=new int[len];
    next[0]=-1;
    for(int i=1;i<100 && t[i]!='\0';i++){
        int j=next[i-1];
        while(t[j+1]!=t[i] && j>=0){
            j=next[j];
        }
        if(t[j+1]==t[i]){
            next[i]=j+1;
        }
        else{
            next[i]=-1;
        }
    }
    return next;
}

int pattern_match_kmp(char *s,char *t,int *next){
    int n=strlen(s);
    int m=strlen(t);
    int p=-1;
    int i=0,j=0;
    while(j<n && i<m){
        if(s[j]==t[i]){
            i++;
            j++;
        }
        else if(i>0){
            i=next[i-1]+1;
        }
        else{
            j++;
        }
    }
    if(i==m){
        p=j-m;
    }
    return p;
}
//KMP算法结束
//三元组压缩矩阵
class compressedmatrix{
    private:
        triple data[150];
        int rpos[30];
        int mu,nu,tu;

    public:
        compressedmatrix(array<array<string,50>,30> &a){
            mu=30;
            nu=50;
            tu=0;
            for(int i=0;i<30;i++){
                rpos[i]=tu;
                for(int j=0;j<50;j++){
                    if(a[i][j]!=""){
                        data[tu].row=i;
                        data[tu].col=j;
                        data[tu].title=a[i][j];
                        tu++;
                    }
                }
            }
        }
        compressedmatrix(){}//默认构造函数
        ~compressedmatrix(){}//析构函数

        triple getdata(int i){
            return data[i];
        }

        int getrpos(int i){
            return rpos[i];
        }

        int getmu(){
            return mu;
        }

        int getnu(){
            return nu;
        }

        int gettu(){
            return tu;
        }

        void getpatent(int i,int j){
            int judge=0;
            for(int k=0;k<tu;k++){
                if(data[k].row==i && data[k].col==j){
                    cout<<"The firm's id is:"<<data[k].row+1001<<" The patent's id is:"<<data[k].col+100001<<" The patent's title is:"<<data[k].title<<endl;
                    judge=1;
                    return;
                }
            }
            if(judge==0){
                cout<<"The firm doesn't have the patent!"<<endl;
            }
            return;
        }

        void key_find(){
            string key;
            cout<<"Please input the key word you want to find:"<<endl;
            fflush(stdin);
            getline(cin,key);
            for(int i=0;i<tu;i++){
                string temp=data[i].title;
                char s[500],t[100];
                strcpy(s,temp.c_str());
                strcpy(t,key.c_str());
                int *next=get_next(t);
                int p=pattern_match_kmp(s,t,next);
                if(p!=-1){
                    cout<<"The key word is founded. The firm's id is:"<<data[i].row+1001<<" The patent's id is:"<<data[i].col+100001<<" The patent's title is:"<<data[i].title<<endl;
                }
            }
            getchar();
            return;
        }
};

class transposedmatrix{
    private:
        triple data[150];
        int rpos[50];
        int mu,nu,tu;
    public:
    transposedmatrix(compressedmatrix a){
        mu=a.getnu();
        nu=a.getmu();
        tu=a.gettu();
        int num_of_columns[50]={0};
        for(int i=0;i<a.gettu();i++){
            num_of_columns[a.getdata(i).col]++;
        }
        int assist[50];
        assist[0]=0;
        for(int i=1;i<50;i++){
            assist[i]=assist[i-1]+num_of_columns[i-1];
        }
        for (int i = 0; i < 50; i++) {
            rpos[i] = assist[i];
        }
        for(int i=0;i<a.gettu();i++){
            int pos=assist[a.getdata(i).col];
            data[pos].row=a.getdata(i).col;
            data[pos].col=a.getdata(i).row;
            data[pos].title=a.getdata(i).title;
            assist[a.getdata(i).col]++;
        }
    }
    ~transposedmatrix(){}

    triple getdata(int i){
            return data[i];
        }

        int getrpos(int i){
            return rpos[i];
        }

        int getmu(){
            return mu;
        }

        int getnu(){
            return nu;
        }

        int gettu(){
            return tu;
        }

    void getpatent(int i,int j){
            int judge=0;
            for(int k=0;k<tu;k++){
                if(data[k].row==i && data[k].col==j){
                    cout<<"The firm's id is:"<<data[k].col+1001<<" The patent's id is:"<<data[k].row+100001<<" The patent's title is:"<<data[k].title<<endl;
                    judge=1;
                    return;
                }
            }
            if(judge==0){
                cout<<"The firm doesn't have the patent!"<<endl;
            }
            return;
        }

    void show_triple(){
        cout<<"row"<<' '<<"col"<<' '<<"title"<<endl;
        for(int i=0;i<tu;i++){
            cout<<data[i].row<<' '<<data[i].col<<' '<<data[i].title<<endl;
        }
        getchar();
        getchar();
        return;
    }
};

void count_shared(compressedmatrix a){//基于矩阵乘法
    int id1,id2;
    int judge=0;
    while(judge==0){
        cout<<"Please input the first firms' id:"<<endl;
        cin>>id1;
        cout<<"Please input the second firms' id:"<<endl;
        cin>>id2;
        if(id1<1001 || id1>1030 || id2<1001 || id2>1030){
            cout<<"Invalid input! Please input again!"<<endl;
        }
        else{
            judge=1;
        }
    }
    int i=id1-1001,j=id2-1001;
    int pos1=a.getrpos(i),pos2=a.getrpos(j);
    int count=0;
    while(a.getdata(pos1).row==i && a.getdata(pos2).row==j && pos1<a.gettu() && pos2<a.gettu()){
        if(a.getdata(pos1).col==a.getdata(pos2).col){
            count++;
            pos1++;
            pos2++;
        }
        else if(a.getdata(pos1).col<a.getdata(pos2).col){
            pos1++;
        }
        else{
            pos2++;
        }
    }
    cout<<"The number of the patents shared by the two firms is:"<<count<<endl;
    getchar();
    getchar();
    return;
}

int main() {
    array<array<string,50>,30> a;
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 50; j++) {
            a[i][j] = "";
        }
    }
    file_to_matrix(a);//将csv文件中的数据存入矩阵a
    compressedmatrix a_com(a);
    transposedmatrix aTra(a_com);
    int judge = 0; // 判断是否退出
    while (judge == 0)
    {
        int choice;
        cout<<"1.Look for patents by key words."<<endl;
        cout<<"2.Show the triple of the transposed matrix"<<endl;
        cout<<"3.Calculate the number of the patents shared by two firms"<<endl;
        cout<<"0.exit"<<endl;
        cout<<"----------------------"<<endl;
        cout<<"Please choose a function:";
        cin>>choice;
        switch (choice)
        {
        case 1:{
            a_com.key_find();
            system("clear");
            break;
        }
        case 2:
            aTra.show_triple();
            system("clear");
            break;
        case 3:
            count_shared(a_com);
            system("clear");
            break;
        case 0:
            judge = 1;
            break;
        default:
            cout<<"Invalid input! Please choose again!"<<endl;
        }
    }
    return 0;
}
//其实就加了一个注释啥都不是就这样混一版
//这里是第一个分支