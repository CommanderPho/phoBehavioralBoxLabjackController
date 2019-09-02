#pragma once
#include <time.h>
#include <chrono>
#include <vector>
#include <map>

#include "LabjackDataFile.h"

typedef std::chrono::system_clock Clock;

//typedef std::tuple<unsigned long long, std::chrono::time_point<Clock>, double> ParsedVariableEventType;

// ParsedVariableEvent represents an event that was parsed from a historical CSV file
// NOTE: THIS IS NOT A Wt EVENT, "Event" refers to a labjack event.
struct ParsedVariableEvent {
	unsigned long long milliseconds_since_epoch;
	std::chrono::time_point<Clock> datetime;
	double labjackValue;

	ParsedVariableEvent(unsigned long long milliseconds_since_epoch, std::chrono::time_point<Clock> datetime, double labjackValue) : milliseconds_since_epoch(milliseconds_since_epoch), datetime(datetime), labjackValue(labjackValue)
	{

	}

	ParsedVariableEvent(std::chrono::time_point<Clock> datetime, double labjackValue) : milliseconds_since_epoch(std::chrono::duration_cast<std::chrono::milliseconds>(datetime.time_since_epoch()).count()), datetime(datetime), labjackValue(labjackValue)
	{

	}

	// Can be called with std::sort(vec.begin(), vec.end(),greater<MyStruct>());
	bool operator > (const ParsedVariableEvent& event) const
	{
		return (milliseconds_since_epoch > event.milliseconds_since_epoch);
	}

	bool operator < (const ParsedVariableEvent& event) const
	{
		return (milliseconds_since_epoch < event.milliseconds_since_epoch);
	}

	// Tries to get the day of the month for the purpose of clustering events by day.
	std::tm getDateDay() {
		std::time_t tt = Clock::to_time_t(this->datetime);
		std::tm local_tm = *localtime(&tt);
		//std::cout << local_tm.tm_year + 1900 << '\n';
		//std::cout << local_tm.tm_mon + 1 << '\n';
		//std::cout << local_tm.tm_mday << '\n';
		local_tm.tm_hour = 0;
		local_tm.tm_min = 0;
		local_tm.tm_sec = 0;
		return local_tm;
	}

	Clock::time_point getDateDayDatetime() {
		std::tm timeinfo = this->getDateDay();
		std::time_t tt = std::mktime(&timeinfo);
		Clock::time_point tp = Clock::from_time_t(tt);
		return tp;
	}

	// returns the number of days ago the event was from now
	int getNumberOfDaysAgo() {
		Clock::duration d = Clock::now() - this->getDateDayDatetime();
		// convert to number of days:
		typedef std::chrono::duration<int, std::ratio<60 * 60 * 24>> days_type;
		days_type ndays = std::chrono::duration_cast<days_type> (d);
		return ndays.count();
	}

};

typedef ParsedVariableEvent ParsedVariableEventType;

struct EventStatistics {

	struct VariableStatistics {
		int numOfEvents = 0;
		int numOfDays = 0;
		std::map<Clock::time_point, int> eventsPerDay;

		VariableStatistics() {

		}

		VariableStatistics(std::vector< ParsedVariableEventType > variableEventsVector) : eventsPerDay(std::map<Clock::time_point, int>()) {
			numOfEvents = variableEventsVector.size();
			for each (auto aVarEvent in variableEventsVector)
			{
				Clock::time_point currDayDatetime = aVarEvent.getDateDayDatetime();
				int currAccumulatedValue = 0;
				if (this->eventsPerDay.count(currDayDatetime) > 0) {
					// "at(...)" is used instead of the traditional index notation ("[...]") because it throws an exception if it doesn't exist instead of adding it silently so we can create the vector if needed.
					try {
						currAccumulatedValue = this->eventsPerDay.at(currDayDatetime);
					}
					catch (...) {
						// Map entry doesn't already exist.
						currAccumulatedValue = 0;
						numOfDays += 1;
					}
				}
				else {
					// Map entry doesn't already exist.
					currAccumulatedValue = 0;
					numOfDays += 1;
				}

				// Add one for the current event
				currAccumulatedValue += 1;
				this->eventsPerDay[currDayDatetime] = currAccumulatedValue;
			}
		}
	};

	EventStatistics() {

	}

	EventStatistics(std::vector< std::vector< ParsedVariableEventType > > variableEventVectors) {
		variableStatsVectors = std::vector<VariableStatistics>();
		for each (auto varEventVector in variableEventVectors)
		{
			VariableStatistics currStats = VariableStatistics(varEventVector);
			this->variableStatsVectors.push_back(currStats);
		}
	}
	// A vector (corresponding to an entry for each variable) of vectors (corresponding to an entry for each event for a given variable) of type ParsedVariableEventType.
	// The ParsedVariableEventType contains the timestamp and the new value at that timestamp
	std::vector<VariableStatistics> variableStatsVectors;

};


// Serves to keep track of the historical data for a specific Behavioral Box. 
// Most specifically the record of Labjack events that have been loaded/parsed from the the .CSV output files
// getHistoricalDataEvents() builds up a vector of event vectors called variableEventVectors
class BehavioralBoxHistoricalData
{
public:

	BehavioralBoxHistoricalData(std::string searchDirectory, int labjackSerialNumber, std::string boxID, std::vector<LabjackDataFile> dataFiles);
	BehavioralBoxHistoricalData(std::string searchDirectory, int labjackSerialNumber, std::vector<LabjackDataFile> dataFiles);

	void getHistoricalDataEvents();
	std::vector<unsigned long long> getFinalMillisecondsSinceEpoch() { return this->output_milliseconds_since_epoch; };
	std::vector<std::vector<double>> getFinalOutputValues() { return this->output_values; };

	int getNumberVariables() { return this->variableEventVectors.size(); };
	std::vector< ParsedVariableEventType > getEvents(int forVariableIndex) { return this->variableEventVectors[forVariableIndex]; };

	// returns the number of events in each variable
	std::vector<int> getNumberEvents();
	// get the maximum number of events any variable has
	int getMaxNumberEvents();
	// Returns a vector of the header strings that were parsed from the files
	std::vector<std::string> getHeaderLabels() { return this->headerLabels_; };
	EventStatistics getEventStatistics() { return this->eventStatistics_; };


private:
	std::string boxID_;
	int labjackSerialNumber_;
	std::string dataFilesSearchDirectory_;

	// A list of data files for that particular box
	std::vector<LabjackDataFile> dataFiles_;

	// Accumulated line timestamps and values for all files:
	std::vector<unsigned long long> milliseconds_since_epoch;
	std::vector<std::vector<double>> values;

	// Parsed outputs
	std::vector<unsigned long long> output_milliseconds_since_epoch;
	std::vector<std::vector<double>> output_values;
	std::vector<std::string> headerLabels_;

	// Variable event outputs: built by getHistoricalDataEvents()
	// A vector (corresponding to an entry for each variable) of vectors (corresponding to an entry for each event for a given variable) of type ParsedVariableEventType.
	// The ParsedVariableEventType contains the timestamp and the new value at that timestamp
	std::vector< std::vector< ParsedVariableEventType > > variableEventVectors;
	EventStatistics eventStatistics_;

	void reloadDataFilesContents();
	void sort() {
		std::sort(dataFiles_.begin(), dataFiles_.end());
	}

};

