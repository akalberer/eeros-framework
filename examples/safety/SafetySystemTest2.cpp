#include <eeros/logger/Logger.hpp>
#include <eeros/logger/StreamLogWriter.hpp>
#include <eeros/safety/SafetySystem.hpp>
#include <eeros/control/TimeDomain.hpp>
#include <eeros/core/Executor.hpp>
#include <eeros/control/Constant.hpp>
#include <eeros/control/I.hpp>
#include <eeros/control/SignalChecker.hpp>

using namespace eeros;
using namespace eeros::hal;
using namespace eeros::safety;
using namespace eeros::logger;
using namespace eeros::control;
using namespace eeros::task;

class ControlSystem;
class SafetyPropertiesTest : public SafetyProperties {
public:
	SafetyPropertiesTest();
	
	ControlSystem* controlSystem;
	double period = 0.01;

	SafetyEvent seStartRampingUp;
	SafetyEvent seReset;
	
	SafetyLevel slStart;
	SafetyLevel slRampingUp;
};

class ControlSystem {
public:
	ControlSystem(SafetySystem& safetySys, SafetyPropertiesTest& ssProperties) : c(0.5), checker(0, 5, safetySys, ssProperties.seReset) {
		i.getOut().getSignal().setName("integrator output");
		i.getIn().connect(c.getOut());
		checker.getIn().connect(i.getOut());
	}

	Constant<> c;
	I<> i;
	SignalChecker<> checker;
};

SafetyPropertiesTest::SafetyPropertiesTest() : 
	seStartRampingUp("start ramping up"),
	seReset("reset"),
	slStart("start"),
	slRampingUp("ramping up")
	{	
	
	// ############ Add levels ############
	addLevel(slStart);
	addLevel(slRampingUp);
	
	// ############ Add events to the levels ############
	slStart.addEvent(seStartRampingUp, slRampingUp, kPrivateEvent);
	slRampingUp.addEvent(seReset, slStart, kPublicEvent);

	// Define and add level functions
	slStart.setLevelAction([&](SafetyContext* privateContext) {
		controlSystem->i.disable();
		controlSystem->i.setInitCondition(0);
		if(slStart.getNofActivations() * period > 3) {
			privateContext->triggerEvent(seStartRampingUp);
			controlSystem->checker.reset();
			controlSystem->i.enable();
		}
	});

	// Define entry level
	setEntryLevel(slStart);	
}

int main() {
	StreamLogWriter w(std::cout);
	Logger::setDefaultWriter(&w);
	Logger log;
	
	log.info() << "Safety System Example 2 started...";
	
	// Create and initialize safety system
	SafetyPropertiesTest ssProperties;
	SafetySystem safetySys(ssProperties, ssProperties.period);
	ControlSystem controlSystem(safetySys, ssProperties);
	ssProperties.controlSystem = &controlSystem;

	TimeDomain td("td1", 0.01, true);
	Periodic periodic("per1", 0.01, td);
	periodic.monitors.push_back([&](PeriodicCounter &pc, Logger &log){
		static int ticks = 0;
		if (++ticks < 100) return;
		ticks = 0;
		log.info() << controlSystem.i.getOut().getSignal();
	});
	
	td.addBlock(controlSystem.c);
	td.addBlock(controlSystem.i);
	td.addBlock(controlSystem.checker);

	// Create and run executor
	auto& executor = eeros::Executor::instance();
	executor.setMainTask(safetySys);
	executor.add(periodic);
	executor.run();

	log.info() << "Test finished...";
}
