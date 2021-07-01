lubertooth = require 'lubertooth'

-- This is a small demo for the minimal LUA bindings to libubertooth.
-- It connects to the first available Ubertooth device and starts the
-- spectrum analyzer mode. It collects 64 samples of the reveiced signal
-- strength indicator (RSSI) at 2,440GHz and reports the average, together
-- with the minimum and maximum.


-- This is the selected frequency.
local usFrequency = 2440

-- Number of samples to collect.
local ulNumberOfSamples = 64

-- This table is used by the callback function to collect all data.
local tMeasure = {
  sum = 0,    -- The sum of all samples.
  cnt = 0,    -- The number of samples already collected.
  min = nil,  -- The current minimum.
  max = nil   -- The current maximum.
}

-- The callback function for the spectrum analyzer. It has 2 arguments:
--   1: the timestamp reported by the device. This seems to be no absolute
--      time, but just some rolling counter.
--   2: a table with a set of frequency->rssi pairs. Note that the rssi values
--      for the frequencies between 2,402GHz and 2,480GHz is delivered in
--      several chunks to the callback function. If you are interested in one
--      frequency, first check if it is part of the delivered data.
-- The function must return 1 boolean value. If the return value is true,
-- sampling will continue. If the return value is false, sampling will stop
-- and the "specan" function call which started the sampling will return.
function callback_specan(dTime, aiRssi)
  -- Get the RSSI value for the selected frequency.
  -- This is nil if the selected frequency is not part of the delivered data chunk.
  local iRssi = aiRssi[usFrequency]
  if iRssi~=nil then
--    print(string.format('@%f  %d = %d', dTime, usFrequency, iRssi))

    -- Add the RSSI value to the sum.
    tMeasure.sum = tMeasure.sum + iRssi
    -- We have one more value.
    tMeasure.cnt = tMeasure.cnt + 1
    -- Update the minimum.
    if tMeasure.min==nil then
      tMeasure.min = iRssi
    elseif iRssi<tMeasure.min then
      tMeasure.min = iRssi
    end
    -- Update the maximum.
    if tMeasure.max==nil then
      tMeasure.max = iRssi
    elseif iRssi>tMeasure.max then
      tMeasure.max = iRssi
    end
  end

  -- Return true if the counter of received samples has not reached the
  -- requested amount. Sampling will continue in this case.
  -- Return false if the counter of received samples reached the requested
  -- amount. Sampling will stop and the "specan" function will return.
  return (tMeasure.cnt<ulNumberOfSamples)
end

-- Display the version of the libraries.
print(string.format('V%s %s', lubertooth.version_libubertooth(), lubertooth.release_libubertooth()))
print(string.format('V%s %s', lubertooth.version_libbtbb(), lubertooth.release_libbtbb()))

-- Create a lubertooth instance.
local tUt = lubertooth.Ubertooth()
-- Connect it to the first available Ubertooth device.
local iResult = tUt:connect(-1)
if iResult<0 then
  print('Failed to open the Ubertooth device.')
else
  -- Start the spectrum alanyzer and deliver data to the callback function
  -- "callback_specan". This function delivers chunks of RSSI values to the
  -- callback function as long as the callback function returns true.
  tUt:specan(callback_specan)
end

-- Print a summary.
if tMeasure.cnt==0 then
  print('No data available.')
else
  print(string.format('Collected %d samples in the range [%d,%d] with an average of %d.', tMeasure.cnt, tMeasure.min, tMeasure.max, tMeasure.sum/tMeasure.cnt))
end
