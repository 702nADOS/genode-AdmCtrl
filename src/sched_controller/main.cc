/*
 * \brief  sched_controller is the user land controller for the kernel schedulers
 * \author Paul Nieleck
 * \date   2016/08/02
 *
 */

/* global includes */
#include <base/env.h>
#include <base/printf.h>
#include <base/rpc_server.h>
#include <base/sleep.h>
//#include <cap_session/connection.h>
#include <base/component.h>
#include <root/component.h>

/* local includes */
#include <sched_controller_session/sched_controller_session.h>
#include <sched_controller/sched_controller.h>
#include "rq_task/rq_task.h"

namespace Sched_controller {

	struct Session_component : Genode::Rpc_object<Session>
	{

		private:
			Sched_controller *_ctr = nullptr;


		public:
			enum { CAP_QUOTA = 2 };
			void get_init_status() {
				PINF("sched_controller is initialized");
			}

			int new_task(Rq_task::Rq_task task, int core)
			{
				return _ctr->enq(core, task);
			}

			void set_sync_ds(Genode::Dataspace_capability ds_cap)
			{
				_ctr->set_sync_ds(ds_cap);
			}

			int are_you_ready()
			{
				return _ctr->are_you_ready();
			}

			int update_rq_buffer(int core)
			{
				return _ctr->update_rq_buffer(core);
			}

			// Optimizer functions
			void optimize (Genode::String<32> task_name)
			{
				_ctr->get_optimizer()->start_optimizing(task_name.string());
			}

			void set_opt_goal (Genode::Ram_dataspace_capability xml_ds_cap)
			{
				_ctr->get_optimizer()->set_goal(xml_ds_cap);
			}
			
			int scheduling_allowed(Genode::String<32> task_name)
			{
				return _ctr->get_optimizer()->scheduling_allowed(task_name.string());
			}
			void last_job_started(Genode::String<32> task_name)
			{
				_ctr->get_optimizer()->last_job_started(task_name.string());
			}
			
			
			/* Session_component constructor enhanced by Sched_controller object */
			Session_component(Sched_controller *ctr)
			: Genode::Rpc_object<Session>()
			{
				_ctr = ctr;
			}
			Session_component(const Session_component&);
			Session_component& operator = (const Session_component&);

	};

	class Root_component : public Genode::Root_component<Session_component>
	{

		private:
			Genode::Env &_env;
	
			Sched_controller *_ctr = nullptr; 
			//Sched_controller *_ctr {_env}; 

		protected:

			//Sched_controller::Session_component *_create_session(const char *args)
			Session_component *_create_session(const char *)
			{
				return new(md_alloc()) Session_component(_ctr);
			}

		public:

			Root_component(Genode::Env       &env, Genode::Entrypoint &ep,
			               Genode::Allocator &allocator,
						   Sched_controller *ctr)
			:Genode::Root_component<Session_component>(ep, allocator), _env(env)
			{
				//PDBG("Creating root component");
				_ctr = ctr;
			}
			Root_component(const Root_component&);
			Root_component& operator = (const Root_component&);

	};

}

//using namespace Genode;

struct Main
{
	Genode::Env &_env;
	Genode::Entrypoint &_ep;

	Sched_controller::Sched_controller ctr {_env};
	//ctr.init_ds(32,2);
	
	//Cap_connection cap;

	Genode::Sliced_heap sliced_heap{_env.ram(),
	                               _env.rm()};

	//enum { STACK_SIZE = 4096 };
	//static Rpc_entrypoint ep(&cap, STACK_SIZE, "sched_controller_ep");

	//static Sched_controller::Root_component sched_controller_root(&ep, &sliced_heap, &ctr);
	//_env.parent().announce(ep.manage(&sched_controller_root));
	
	Sched_controller::Root_component _sched_controller_root{_env, _ep, sliced_heap, &ctr};
	Main(Genode::Env &env) : _env(env), _ep(_env.ep())
	{
		ctr.init_ds(32,2);	
		_env.parent().announce(_ep.manage(_sched_controller_root));		
	}

};

void Component::construct(Genode::Env &env) { static Main main(env); }
	
