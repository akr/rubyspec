require File.dirname(__FILE__) + '/../../spec_helper'

describe "IO#close" do
  before :each do
    @io = File.open tmp('io.close.txt'), 'w'
  end

  after :each do
    @io.close unless @io.closed?
  end

  it "closes the stream" do
    lambda { @io.close }.should_not raise_error
    @io.closed?.should == true
  end

  it "returns nil" do
    @io.close.should == nil
  end

  it "makes the stream unavailable for any further data operations" do
    @io.close
    lambda { @io.print "attempt to write" }.should raise_error(IOError)
    lambda { @io.syswrite "attempt to write" }.should raise_error(IOError)
    lambda { @io.read }.should raise_error(IOError)
    lambda { @io.sysread(1) }.should raise_error(IOError)
  end

  it "raises an IOError on subsequent invocations" do
    @io.close
    lambda { @io.close }.should raise_error(IOError)
    lambda { @io.close }.should raise_error(IOError)
  end

end

describe "IO#close on an IO.popen stream" do

  it "clears #pid" do
    io = IO.popen 'yes', 'r'

    io.pid.should_not == 0

    io.close

    lambda { io.pid }.should raise_error(IOError, 'closed stream')
  end

  it "sets $?" do
    io = IO.popen 'true', 'r'
    io.close

    $?.exitstatus.should == 0

    io = IO.popen 'false', 'r'
    io.close

    $?.exitstatus.should == 1
  end

  it "waits for the child to exit" do
    io = IO.popen 'yes', 'r'
    io.close

    $?.exitstatus.should_not == 0 # SIGPIPE/EPIPE
  end

end

