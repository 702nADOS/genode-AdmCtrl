/*
 * \brief  sched_controller
 * \author Paul Nieleck
 * \date   2016/09/09
 *
 */

/* for testing in get_utilization   */
#include <random>
#include <timer_session/connection.h>
#include <string>
/* ******************************** */

#include <unordered_map>
#include <base/printf.h>

#include "sched_controller/sched_controller.h"
#include "sched_controller/task_allocator.h"
#include "rq_manager_session/client.h"
#include "rq_manager_session/connection.h"

namespace Sched_controller {

	/**
	 * Get and set the number of available physically
	 * available CPU cores of the system.
	 * This function will not change the number of
	 * _pcore objects.
	 *
	 * \return success status
	 */
	int Sched_controller::_set_num_pcores()
	{
		/* 
		 * From the monitor we will request the number of physical
		 * cores available at the system. Currently this feature is
		 * not implemented, therefore we will set the number of cores
		 * to be 4.
		 */
		_num_pcores = 4;

		return 0;
	}

	/**
	 * Initialize the pcores, i.e. create new
	 * instances of the pcore class
	 *
	 * \return success status
	 */
	int Sched_controller::_init_pcores()
	{

		_pcore = new Pcore[_num_pcores];

		for (int i = 0; i < _num_pcores; i++) {
			_pcore[i].set_id(i);
		}

		return 0;
	}

	/**
	 * Initialize the run queues
	 *
	 * \return success status
	 */
	int Sched_controller::_init_runqueues()
	{

		/* currently this is pretty stupid and
		 * not what we actually want.
		 */

		/* For the final implementation it is actually planned
		 * to initialize the run queues dynamically, but therefore
		 * the rq_manager has to be changed accordingly. At the
		 * moment the rq_manager is configured to provide a fixed
		 * number of run queues.
		 */
		_num_rqs = _rq_manager.get_num_rqs();
		PDBG("Number of supplied run queues is: %d", _num_rqs);

		_runqueue = new Runqueue[_num_rqs];

		for (int i = 0; i < _num_pcores; i++) {
			_runqueue[i]._task_class = Rq_manager::Task_class::lo;
			_runqueue[i]._task_strategy = Rq_manager::Task_strategy::priority;
			_runqueue[i].rq_buffer = i;
		}

		return 0;
	}

	/**
	 * Call the Task_allocator to allocate newly arriving tasks
	 * (comming in via the respective RPC-call) to a sufficient
	 * pcore/rq_buffer.
	 *
	 * \param newly arriving task
	 */
	void Sched_controller::allocate_task(Rq_manager::Rq_task task)
	{

		PINF("Now we'll allocate Task with id %d", task.task_id);
		Task_allocator::allocate_task(this, &task);

	}

	void Sched_controller::task_to_rq(int rq, Rq_manager::Rq_task *task) {
		int status = _rq_manager.enq(rq, *task);
		PDBG("%d", status);
		return;
	}

	/**
	 *
	 */
	int Sched_controller::get_num_rqs()
	{
		return _num_rqs;
	}

	/**
	 * Return the run queues that support the requested task_class
	 * and task_strategy.
	 *
	 * \param
	 *
	 */
	void Sched_controller::which_runqueues(std::vector<Runqueue> *rq, Rq_manager::Task_class task_class, Rq_manager::Task_strategy task_strategy)
	{
		//rq = new std::vector<Runqueue>;
		rq->reserve(_num_rqs);
		for (int i = 0; i < _num_rqs; i++) {
			if (_runqueue[i]._task_class == task_class) {
				if (_runqueue[i]._task_strategy == task_strategy) {
					rq->push_back(_runqueue[i]);
				}
			}
		}

		return;
	}

	double Sched_controller::get_utilization(int rq) {
		/* This is a mock, only for testing purposes */

		Timer::Connection _timer;
		unsigned long time_ms = _timer.elapsed_ms();

		std::string seed_str = std::to_string(time_ms);
		PINF("The elapsed time for the seed is: %ld", time_ms);

		std::default_random_engine generator;
		std::seed_seq seed1 (seed_str.begin(),seed_str.end());

		generator.seed(seed1);
		std::uniform_real_distribution<double> distribution(0.0,1.0);

		return distribution(generator);
	}

	/******************
	 ** Constructors **
	 ******************/

	Sched_controller::Sched_controller()
	{

		/* We then need to figure out how many CPU cores are available at the system */
		_set_num_pcores();

		/* And finally we will create instances of _pcore */
		_init_pcores();

		/* Now lets create the runqueues we're working with */
		_init_runqueues();

		/*
		 * After we know about our run queues, we will assign them to the pcores.
		 * Currently we have 4 run queues and 4 pcores. Hence we can make a fixed
		 * assignement.
		 */
		for (int i = 0; i < _num_rqs; i++) {
			std::pair<Pcore*, Runqueue*> _pcore_rq_pair (_pcore + i, _runqueue + i);
			_pcore_rq_association.insert(_pcore_rq_pair);
			PINF("Allocated rq_buffer %d to _pcore %d", i, i);
		}

	}

	Sched_controller::~Sched_controller()
	{

	}

}