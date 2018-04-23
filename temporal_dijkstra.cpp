#include<stdio.h>
#include<string>
#include<vector>
#include<iostream>
#include<math.h>
#include"utility1.h"
#include"head.h"

using namespace std;
const int Total_Timestamp=1440;
const int VectorDefaultSize = 20;
const double epsilon = 1e-5;

struct edge 
{
	int source;
	int end;
	iVector<pair<int,int> > ft; //same as time_info in graph_generate.cpp
	int ft_value(int t)
	{
		t=t%Total_Timestamp;
		if(ft.size()==1)
		{
			return (ft[0].second);
		}
		if(t==0)
		{
			if(ft[0].first!=0)
				return ft[ft.size()-1].second;
			else
				return ft[0].second;
		}
		bool flag=false;
		int i;
		for(i=0;i<ft.size();i++)
		{
			if(ft[i].first<t)
				continue;
			else
			{
				flag=true;
				break;
			}
		}
			
		double x1,y1,x2,y2;
		if(i==0||i==ft.size())
		{
			y1=ft[ft.size()-1].second;
			x1=ft[ft.size()-1].first-1440;
			i=0;
		}
		else
		{
			y1=ft[i-1].second;
			x1=ft[i-1].first;
		}
		y2=ft[i].second;
		x2=ft[i].first;
		int ret;
		if(abs(x2-x1)<=epsilon)	
		{
			ret=(int)((y1+y2)/2);
		}
		else
		{
			ret= (int)((y2-y1)/(x2-x1)*(t-x1)+y1);
		}
		if(ret<0)
		{
			printf("u=%d,v=%d,time=%d,x1=%lf,y1=%lf,x2=%lf,y2=%lf,ret=%d\n",source,end,t,x1,y1,x2,y2,ret);
			getchar();
		}
		return ret;
	}
	int ft_dis(int t)
	{
		return ft_value(t)+t;
	}
	void print()
	{
		printf("source:%d\tend:%d\tft_size:%d\n",source, end, ft.size());
		for(int i=0;i<ft.size();i++)
		{
			printf("%d\t%d\n",ft[i].first, ft[i].second);
		}
		printf("\n");
	}
};

struct distance_label
{
	int source;
	int end;
	iVector<pair<int,int> > distance; //distance label for profile dijkstra
	int min_d; //the lower bound of distance label
	int infty;  //infinity
	
	distance_label(int s, int t)
	{
		infty=-9;
		source=s;
		end=t;
		min_d=infty;
		if(s==t)
		{
			min_d=0;
		}
	}
	distance_label()
	{
		infty=-9;
		source=infty;
		end=infty;
		min_d=infty;
	}
	distance_label(edge e)
	{
		infty=-9;
		source=e.source;
		end=e.end;
		distance=e.ft;
		min_d_reset();
	}
	void min_d_reset() // the function set min_d according to this.distance
	{
		min_d=infty;
		for(int i=0;i<distance.size();i++)
		{
			if(min_d==infty)
				min_d=distance[i].second;
			else if(min_d>distance[i].second)
				min_d=distance[i].second;
		}
	}
	int dis_value(int t)
	{
		t=t%Total_Timestamp;
		if(distance.size()==0)
			return 0;
		if(distance.size()==1)
		{
			return distance[0].second;
		}
		if(t==0)
		{	
			if(distance[0].first!=0)
				return distance[distance.size()-1].second;
			else
				return distance[0].second;
		}
		bool flag=false;
		int i;
		for(i=0;i<distance.size();i++)
		{
			if(distance[i].first<t)
				continue;
			else
			{
				flag=true;
				break;
			}
		}
			
		double x1,y1,x2,y2;
		if(i==0||i==distance.size())
		{
			y1=distance[distance.size()-1].second;
			x1=0;
			i=0;
		}
		else
		{
			y1=distance[i-1].second;
			x1=distance[i-1].first;
		}
		y2=distance[i].second;
		x2=distance[i].first;
		int ret= (int)((y2-y1)/(x2-x1)*(t-x1)+y1);
		if(ret<0)
		{
			printf("u=%d,v=%d,time=%d,x1=%lf,y1=%lf,x2=%lf,y2=%lf,ret=%d\n",source,end,t,x1,y1,x2,y2,ret);
			getchar();
		}
		return ret;
	}
	int dis_dis(int t)
	{
		return dis_value(t)+t;
	}
	bool operator<(const distance_label & dis)
	{	
		if(min_d==infty)
			return false;
		if(min_d<dis.min_d && min_d>=0)
			return true;
		return false;
	}
	bool operator==(const distance_label & dis)
	{
		return (min_d==dis.min_d);
	}
	bool operator==(const int & x)
	{
		return (min_d==x);
	}
	distance_label & operator=(const int &x)
	{
		if(x==-9)
		{
			source=-9;
			end=-9;
			infty=-9;
		}
		return *this;
	}
	/*
	distance operator+(const distance_label & dis)
	{
		if(distance.size()==0)
			return dis;
		int i=0,j=0;
		while(i<distance.size()&&j<dis.distance.size())
		{
			
		}
	}
	*/
	void insert(int timestamp, int dis)
	{
		distance.push_back(pair<int,int>(timestamp, dis));
		if(min_d>dis)
			min_d=dis;
	}
	void print()
	{
		printf("source:%d\tend:%d\tmin_d:%d\tinfty:%d\tdistance_size:%d\n",source, end, min_d, infty, distance.size());
		for(int i=0;i<distance.size();i++)
		{
			printf("%d\t%d\n",distance[i].first, distance[i].second);
		}
		printf("\n");
	}
};

int n;


distance_label function_add(distance_label d, edge e)
{
//addition of functions 
	distance_label ret;
	ret.source=d.source;
	ret.end=e.end;
	int i=0,j=0;
	//printf("functions add start\nd is \n");
	//d.print();
	//printf("e is \n");
	//e.print();
	while(i<d.distance.size()||j<e.ft.size())
	{
		int push_first,push_second;
		if(d.distance[i].first==e.ft[j].first)
		{
			push_first=d.distance[i].first;
			push_second=d.distance[i].second+e.ft[j].second;
			//printf("case 1: push_first=%d,push_second=%d\n",push_first,push_second);
			i++;
			j++;
		}
		else if(d.distance[i].first<e.ft[j].first)
		{
			push_first=d.distance[i].first;
			push_second=d.distance[i].second+e.ft_value(push_first);
			//printf("case 2: push_first=%d,push_second=%d\n",push_first,push_second);
			i++;
		}
		else if(d.distance[i].first>e.ft[j].first)
		{
			push_first=e.ft[j].first;
			push_second=d.dis_value(push_first)+e.ft[j].second;
			//printf("case 3: push_first=%d,push_second=%d\n",push_first,push_second);
			j++;
		}
		//printf("i=%d,push_first=%d,push_second=%d\n",i,push_first,push_second);
		pair<int,int> push_item(push_first,push_second);
		//printf("push_item:%d,%d\n",push_item.first,push_item.second);
		//ret.distance.push_back(pair<int,int>(push_first,push_second));
		//printf("before pushed, ret=\n");
		//ret.print();
		ret.distance.push_back(push_item);
		//printf("after pushed, ret=\n");
		//ret.print();
	}
	//printf("intermidate ret is \n");
	//ret.print();
	//printf("i=%d\tj=\%d\n",i,j);
	if(i==d.distance.size()&&j<e.ft.size())
	{
		int push_first,push_second;
		while(j<e.ft.size())
		{
			push_first=e.ft[j].first;
			push_second=d.dis_value(push_first)+e.ft[j].second;
			j++;
		}	
		ret.distance.push_back(pair<int,int>(push_first,push_second));
	}
	else if(j==e.ft.size()&&i<d.distance.size())
	{
		int push_first,push_second;
		while(i<d.distance.size())
		{
			push_first=d.distance[i].first;
			push_second=d.distance[i].second+e.ft_value(push_first);
			i++;	
		}
		ret.distance.push_back(pair<int,int>(push_first,push_second));
	}
	//printf("Function add succeed!\nThe sum is \n");
	//ret.print();

	return ret;

}

edge function_composition(distance_label d, edge e)
{
//return e.ft_value(d.dis_dis()), aka f(g(t)+t)
	int i=0,j=0;
	edge ret;
	while(i<d.distance.size())
	{
		int push_first,push_second;
		push_first=d.distance[i].first;
		push_second=e.ft_value(d.distance[i].second+push_first);
		ret.ft.push_back(pair<int,int>(push_first,push_second));
		i++;
	}
	ret.source=d.source;
	ret.end=e.end;
	//printf("functions composition suceed!\n");
	//ret.print();
	return ret;
}
distance_label function_oplus(distance_label d, edge e) //oplus operation f(t) oplus g(t)=f(g(t)+t)+g(t)
{
	distance_label ret;
	if(d.distance.size()==0)
		return distance_label(e);
	ret=function_add(d,function_composition(d,e));
	return ret;

}
void edge_ft_print(edge e) // print an edge as well as its f(t) pieces
{
	printf("%d\t%d:",e.source,e.end);
	for(int i=0;i<e.ft.size();i++)
	{
		printf("(%d,%d)\t",e.ft[i].first,e.ft[i].second);
	}
	printf("\n");
}

void attached_edge_print(iVector<edge> attached_edges) // print ingoing or outgoing edges for a node
{
	for(int i =0; i<attached_edges.size();i++)
	{
		edge_ft_print(attached_edges[i]);
	}
}
void edge_set_print(iVector<edge> edges_set[]) // print all in_edges or out_edges of graph G
{
	for(int i=0;i<n;i++)
	{
		attached_edge_print(edges_set[i]);
	}
}
void edge_set_print(iVector<iVector<edge> > edges_set) 
{
	for(int i=0;i<n;i++)
	{
		attached_edge_print(edges_set[i]);

	}
}

iHeap<int> djk_heap;
iHeap<distance_label> pro_djk_heap;
iMap<int> djk_map;
iMap<distance_label> pro_djk_map;

void profile_dijkstra(iVector<iVector<edge> > out_edges, int previous[],int s=0, int time=0)
{
	distance_label label_ss(s,s); //distance label from source to source
	pro_djk_map.initialize(n);
	pro_djk_map.insert(s,label_ss);
	pro_djk_heap.initialize(n);
	pro_djk_heap.insert(s,label_ss);
	Timer * timer=new Timer(1);
	long cnt=0;
	while(!pro_djk_heap.empty())
	{
		/*if(cnt++%100000==0)
		{
			printf("cnt=%ld\n",cnt);
		}
		*/
		int v=pro_djk_heap.head();
		//printf("poped v=%d,value=%d\n",v,djk_map[v]);
		pro_djk_heap.pop();
		//printf("poppd_node:%d\n", v);
		distance_label temp_d = pro_djk_map[v];
		//if(temp_d.distance.size()>2||out_edges[v].size()>10)
		{
			//temp_d.print();
			//getchar();
		}
		for(int i=0;i<out_edges[v].size();i++)
		{
			edge e=out_edges[v][i];
			//printf("i=%d\n",i);
			//e.print();
			//getchar();
			/*
			if(e.source==406181&&e.end==406180)
			{
				continue;
			}
			*/
			if(pro_djk_map.notexist(e.end))
			{
				//int time_u=pro_djk_map[v]+e.ft_dis(pro_djk_map[v]);
				//printf("new label insert start!\n");
				//printf("u=%d,time_u=%d\n",e.end,time_u);
				//djk_map.insert(e.end,time_u);
				//djk_heap.insert(e.end,time_u);
				//previous[e.end]=v;
				distance_label new_d;
				//printf("oplus funciton g\n");
				//pro_djk_map[v].print();
				new_d=function_oplus(pro_djk_map[v],e);
				//printf("oplus suceed!\n");
				new_d.min_d_reset();
				pro_djk_map.insert(e.end, new_d);
				pro_djk_heap.insert(e.end,new_d);
				//printf("new label insert\n");
				//new_d.print();
			}
			else
			{       /*
				int time_u=djk_map[v]+e.ft_dis(djk_map[v]);
				if(time_u<djk_map[e.end])
				{
					//printf("u=%d,time_u=%d\n",e.end,time_u);
					djk_map.insert(e.end,time_u);
					djk_heap.insert(e.end,time_u);
					previous[e.end]=v;
				}
				*/
				//printf("label update start\n");
				distance_label new_d;
				
				new_d=function_oplus(pro_djk_map[v],e);
				new_d.min_d_reset();
				if(new_d<pro_djk_map[e.end])
				{
					//pro_djk_map.erase(e.end);
					pro_djk_map.insert(e.end, new_d);
					pro_djk_heap.insert(e.end, new_d);
					//if(new_d.distance.size()>2)
					{
						//printf("new label update\n");
						//new_d.print();
						//getchar();
					}
				}
				//else
				{
					//printf("no need udpate\n");
				}
			}
			//printf("!!!i=%d,size=%d\n",i,out_edges[i].size());	
		}
	}
	delete timer;
}

void temporal_dijkstra(iVector<iVector<edge> > out_edges, int previous[],int s=0, int time=0)
{
	djk_map.initialize(n);
	djk_map.insert(s,time);
	djk_heap.initialize(n);
	djk_heap.insert(s,time);
	while(!djk_heap.empty())
	{
		int v=djk_heap.head();
		//printf("poped v=%d,value=%d\n",v,djk_map[v]);
		djk_heap.pop();
		for(int i=0;i<out_edges[v].size();i++)
		{
			edge e=out_edges[v][i];
			if(djk_map.notexist(e.end))
			{
				int time_u=djk_map[v]+e.ft_dis(djk_map[v]);
				//printf("u=%d,time_u=%d\n",e.end,time_u);
				djk_map.insert(e.end,time_u);
				djk_heap.insert(e.end,time_u);
				previous[e.end]=v;
			}
			else
			{
				int time_u=djk_map[v]+e.ft_dis(djk_map[v]);
				if(time_u<djk_map[e.end])
				{
					//printf("u=%d,time_u=%d\n",e.end,time_u);
					djk_map.insert(e.end,time_u);
					djk_heap.insert(e.end,time_u);
					previous[e.end]=v;
				}
			}
			
		}
	}
	
}
void map_print(iMap<int> temp_map=djk_map)
{
	for(int i=0;i<n;i++)
	{
		printf("i=%d,time=%d\n",i,temp_map[i]);
	}
}
int main()
{
	FILE * fin;
	//fin=fopen("../sample_graph.in","r");
	fin=fopen("../BeijingTemporal.out","r");

	if(fin == NULL)
		printf("invalid file address\n");
	fscanf(fin,"%d",&n);
	printf("%d\n",n);
	//char temp='!';
	const int N=n;
	//edge out_edge[N];
	iVector<iVector<edge> > out_edges(n);
	iVector<iVector<edge> >in_edges(n);
	int previous[N];
	//begin reading the graph
	for(int i=0;i<n;i++)
	{
		int u,v;
		fscanf(fin,"%d",&u);
		fscanf(fin,"%d",&v);
		if(i%100000==0)
			printf("i=%d\n",i);
		while(1)
		{
			edge temp_e;
			temp_e.source=u;
			temp_e.end=v;
			int k=0;
			fscanf(fin,"%d",&k);
			for(int j=0;j<k;j++)
			{
				int t, ft;
				fscanf(fin,"%d",&t);
				fscanf(fin,"%d",&ft);
				temp_e.ft.push_back(pair<int,int>(t,ft));
				//push each pieces of f(t) into edge.ft
			}
			temp_e.ft.push_back(pair<int,int>(1440, temp_e.ft[0].second));
			//push the f(1440) into the iVector, whose value is same as f(0)
			out_edges[u].push_back(temp_e);
			in_edges[v].push_back(temp_e);
			fscanf(fin,"%d",&v);
			if(v==-1)
				break;

		}


		
	}
	fclose(fin);
	//temporal_dijkstra(out_edges,previous,0,0);
	clock_t start_time,end_time;
	start_time=clock();	
	profile_dijkstra(out_edges,previous,0,0);
	end_time=clock();
	double totol_time=(double)(end_time-start_time)/CLOCKS_PER_SEC;
	double time_per_node=totol_time*1000/N;
	//printf("Total Time of process is %0.2lf s,time_per_node is %0.2lf ms\n",totol_time,time_per_node);
	//map_print(djk_map);
	//edge_set_print(out_edges);
	//edge_set_print(in_edges);
	//delete [] out_edges;
	//delete [] in_edges;
	return 0;	
}
