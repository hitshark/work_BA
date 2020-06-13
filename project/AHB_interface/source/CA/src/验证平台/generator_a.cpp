#include "generator_a.h"

SC_HAS_PROCESS(generator_a);
generator_a::generator_a(sc_core::sc_module_name nm
                     ,unsigned int start_address
                     ,unsigned int end_address
                    )
:sc_module(nm)
,start_addr(start_address)
,end_addr(end_address)
{
    data=0;
    SC_THREAD(generate_thread);
    //dont_initialize();
}

void generator_a::generate_thread()
{
	bool status;
    sc_core::wait(40,sc_core::SC_NS);

	// single read and write
    for(unsigned char i=65;i<74;i++)
    {
		unsigned char return_data;
        wr_port->write_byte(start_addr+20,i);
		//cout<<"write byte data is:"<<i<<endl;
        return_data=wr_port->read_byte(start_addr+20);
        start_addr=start_addr+1;
        wait(10,sc_core::SC_NS);
        cout<<"read byte data:	"<<return_data<<"		at time:"<<sc_time_stamp()<<endl;
    }
	start_addr=0x2000000;
    for(unsigned short i=65;i<74;i++)
    {
        unsigned short return_data;
        wr_port->write_half(start_addr+1024,i);
		//cout<<"write half data is:"<<i<<endl;
        return_data=wr_port->read_half(start_addr+1024);
        start_addr=start_addr+2;
        wait(10,sc_core::SC_NS);
        cout<<"read half data:"<<return_data<<"	at time:"<<sc_time_stamp()<<endl;
    }
	start_addr=0x2000000;
	for(unsigned int i=65;i<74;i++)
    {
        unsigned int return_data;
        wr_port->write(start_addr+1024,i);
		//cout<<"write data is:"<<i<<endl;
        return_data=wr_port->read(start_addr+1024);
        start_addr=start_addr+4;
        wait(10,sc_core::SC_NS);
        cout<<"read data:"<<return_data<<"	at time:"<<sc_time_stamp()<<endl;
    }

	// burst read and write while the address is not wrapped
    unsigned char *w_data=new unsigned char[16];
	unsigned char *temp;
	temp=w_data;
	unsigned int addr=0x2000000;
	for(unsigned char i=65;i<81;i++)
	{
		*temp=i;
		temp++;
	}

    unsigned char *r_data=new unsigned char [32];

    status=wr_port->incr_burst_write(addr+100,w_data,16,1);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		//cout<<*w_data<<endl;
		temp=w_data;
		for(int i=0;i<16;i++)
		{
			cout<<*temp<<"    ";
			temp++;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->incr_burst_read(addr+100,r_data,16,1);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i++)
        {
            cout<<*temp<<"    ";
            temp++;
        }
        cout<<endl;
    }

	temp=w_data;
    status=wr_port->incr_burst_write(addr+996,w_data,16,2);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		for(int i=0;i<16;i=i+2)
		{
			cout<<(*reinterpret_cast<unsigned short int *>(temp))<<"    ";
			temp=temp+2;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->incr_burst_read(addr+996,r_data,16,2);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i=i+2)
        {
            cout<<(*reinterpret_cast<unsigned short int *>(temp))<<"    ";
            temp=temp+2;
        }
        cout<<endl;
    }

	temp=w_data;
    status=wr_port->incr_burst_write(addr+20,w_data,16,4);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		for(int i=0;i<16;i=i+4)
		{
			cout<<(*reinterpret_cast<unsigned int *>(temp))<<"    ";
			temp=temp+4;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->incr_burst_read(addr+20,r_data,16,4);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i=i+4)
        {
            cout<<(*reinterpret_cast<unsigned int *>(temp))<<"    ";
            temp=temp+4;
        }
        cout<<endl;
    }

	// burst read and write while the address is wrapped!
	status=wr_port->wrap_burst_write(addr+20,w_data,16,1);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		//cout<<*w_data<<endl;
		temp=w_data;
		for(int i=0;i<16;i++)
		{
			cout<<*temp<<"    ";
			temp++;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->wrap_burst_read(addr+20,r_data,16,1);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i++)
        {
            cout<<*temp<<"    ";
            temp++;
        }
        cout<<endl;
    }

	temp=w_data;
    status=wr_port->wrap_burst_write(addr+20,w_data,16,2);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		for(int i=0;i<16;i=i+2)
		{
			cout<<(*reinterpret_cast<unsigned short int *>(temp))<<"    ";
			temp=temp+2;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->wrap_burst_read(addr+20,r_data,16,2);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i=i+2)
        {
            cout<<(*reinterpret_cast<unsigned short int *>(temp))<<"    ";
            temp=temp+2;
        }
        cout<<endl;
    }

	temp=w_data;
    status=wr_port->wrap_burst_write(addr+20,w_data,16,4);
	if(status)
	{
		cout<<"at time:"<<sc_time_stamp()<<"  write data is:"<<endl;
		for(int i=0;i<16;i=i+4)
		{
			cout<<(*reinterpret_cast<unsigned int *>(temp))<<"    ";
			temp=temp+4;
		}
		cout<<endl;
	}

	temp=r_data;
    status=wr_port->wrap_burst_read(addr+20,r_data,16,4);
    if(status==true)
    {
        cout<<"at time:"<<sc_time_stamp()<<"  read data:"<<endl;
        for(int i=0;i<16;i=i+4)
        {
            cout<<(*reinterpret_cast<unsigned int *>(temp))<<"    ";
            temp=temp+4;
        }
        cout<<endl;
    }
	delete [] w_data;
	delete [] r_data;
}
