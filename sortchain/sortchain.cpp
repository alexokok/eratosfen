/*$TET$actor*/
/*--------------------------------------------------------------------------*/
/*  Copyright 2016 Sergei Vostokin                                          */
/*                                                                          */
/*  Licensed under the Apache License, Version 2.0 (the "License");         */
/*  you may not use this file except in compliance with the License.        */
/*  You may obtain a copy of the License at                                 */
/*                                                                          */
/*  http://www.apache.org/licenses/LICENSE-2.0                              */
/*                                                                          */
/*  Unless required by applicable law or agreed to in writing, software     */
/*  distributed under the License is distributed on an "AS IS" BASIS,       */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*/
/*  See the License for the specific language governing permissions and     */
/*  limitations under the License.                                          */
/*--------------------------------------------------------------------------*/

#include <iostream>
#include <templet.hpp>

using namespace std;


const int N = 15;

int arr[N];
/*$TET$*/

using namespace TEMPLET;

///какой-то движок
struct my_engine : engine{
	my_engine(int argc, char *argv[]){
		::init(this, argc, argv);
	}
	void run(){ TEMPLET::run(this); }
	void map(){ TEMPLET::map(this); }
};

enum MESSAGE_TAGS{ START };

#pragma templet ~mes=

struct mes : message{
	mes(actor*a, engine*e, int t) : _where(CLI), _cli(a), _client_id(t){
		::init(this, a, e);
	}

	bool access(actor*a){
		return TEMPLET::access(this, a);
	}

	void send(){
		if (_where == CLI){ TEMPLET::send(this, _srv, _server_id); _where = SRV; }
		else if (_where == SRV){ TEMPLET::send(this, _cli, _client_id); _where = CLI; }
	}

/*$TET$mes$$data*/
	int number;
/*$TET$*/

	enum { CLI, SRV } _where;
	actor* _srv;
	actor* _cli;
	int _client_id;
	int _server_id;
};

#pragma templet *producer(out!mes)+

///актор, запускающий алгоритм сортировки
struct producer : actor{
	enum tag{TAG_out=START+1};

	producer(my_engine&e):_out(this, &e, TAG_out){
		::init(this, &e, producer_recv_adapter);
		::init(&_start, this, &e);
		::send(&_start, this, START);
/*$TET$producer$producer*/
/*$TET$*/
	}

	void at(int _at){ TEMPLET::at(this, _at); }
	void delay(double t){ TEMPLET::delay(this, t); }
	double time(){ return TEMPLET::time(this); }
	void stop(){ TEMPLET::stop(this); }

	mes* out(){return &_out;}

	static void producer_recv_adapter (actor*a, message*m, int tag){
		switch(tag){
			case TAG_out: ((producer*)a)->out(*((mes*)m)); break;
			case START: ((producer*)a)->start(); break;
		}
	}

	void start(){
/*$TET$producer$start*/
		cur = 0;
		_out.number = arr[cur++];
		_out.send();
/*$TET$*/
	}

	void out(mes&m){
/*$TET$producer$out*/
		if (cur == N) return;
		_out.number = arr[cur++];
		_out.send();
/*$TET$*/
	}

/*$TET$producer$$code&data*/
	int cur;
/*$TET$*/

	mes _out;
	message _start;
};

#pragma templet *sorter(in?mes,out!mes)

///сортировщик, будущее решето
struct sorter : actor{
	enum tag{TAG_in,TAG_out};

	sorter(my_engine&e):_out(this, &e, TAG_out){
		::init(this, &e, sorter_recv_adapter);
/*$TET$sorter$sorter*/
		is_first=true;
		_in=0;
/*$TET$*/
	}

	void at(int _at){ TEMPLET::at(this, _at); }
	void delay(double t){ TEMPLET::delay(this, t); }
	double time(){ return TEMPLET::time(this); }
	void stop(){ TEMPLET::stop(this); }

	void in(mes*m){m->_server_id=TAG_in; m->_srv=this;}
	mes* out(){return &_out;}

	static void sorter_recv_adapter (actor*a, message*m, int tag){
		switch(tag){
			case TAG_in: ((sorter*)a)->in(*((mes*)m)); break;
			case TAG_out: ((sorter*)a)->out(*((mes*)m)); break;
		}
	}

	void in(mes&m){
/*$TET$sorter$in*/
		_in=&m;
		sort();
/*$TET$*/
	}

	void out(mes&m){
/*$TET$sorter$out*/
		sort();
/*$TET$*/
	}

/*$TET$sorter$$code&data*/
	void sort() {
		///проверка на то, что сообщение пришло и ушло
		if (!(_in->access(this) && _out.access(this)))return;

		if (is_first) {
			number = _in->number;
			is_first = false;
			_in->send();
		}
		else {
			int count = 0;

			for (int i = 2; i < _in->number - 1; i++) {
				if (_in->number % i == 0)
					count++;
			}
			if (count == 0) {
				_out.number = _in->number;
				_in->send(); _out.send();
			}
			else {
				_out.number = 0;
				_in->send(); _out.send();
			}
		}		
	}

	int number;
	bool is_first;
	mes* _in;
/*$TET$*/

	mes _out;
};

#pragma templet *stoper(in?mes)

struct stoper : actor{
	enum tag{TAG_in};

	stoper(my_engine&e){
		::init(this, &e, stoper_recv_adapter);
/*$TET$stoper$stoper*/
/*$TET$*/
	}

	void at(int _at){ TEMPLET::at(this, _at); }
	void delay(double t){ TEMPLET::delay(this, t); }
	double time(){ return TEMPLET::time(this); }
	void stop(){ TEMPLET::stop(this); }

	void in(mes*m){m->_server_id=TAG_in; m->_srv=this;}

	static void stoper_recv_adapter (actor*a, message*m, int tag){
		switch(tag){
			case TAG_in: ((stoper*)a)->in(*((mes*)m)); break;
		}
	}

	void in(mes&m){
/*$TET$stoper$in*/
		number=m.number;
		stop();
/*$TET$*/
	}

/*$TET$stoper$$code&data*/
	int number;
/*$TET$*/

};

/*$TET$footer*/
int main(int argc, char *argv[])
{
	my_engine e(argc, argv);
	
	sorter** a_sorter = new sorter*[N-1];
	for(int i=0;i<N-1;i++)a_sorter[i]=new sorter(e);

	producer a_producer(e);
	stoper an_endpoint(e);

	mes* prev=a_producer.out();
	for(int i=0;i<N-1;i++){
		a_sorter[i]->in(prev);
		prev=a_sorter[i]->out();
	}
	an_endpoint.in(prev);
	
	for(int i=0;i<N;i++) 
		arr[i]=i;
	
	printf("before the sort: \n");
	
	for (int i = 0; i < N; i++)
		printf("%d\n", arr[i]);

	e.run();
	
	for(int i=0;i<N-1;i++){
		arr[i]=a_sorter[i]->number;
	}

	arr[N-1]=an_endpoint.number;
	
	printf("\n\nafter the sort: \n");
	
	for (int i = 0; i < N; i++) {

		if(arr[i] != 0)
			printf("%d\n", arr[i]);
	}

	int x = 0;

	cin >> x;

	return 0;
}
/*$TET$*/
