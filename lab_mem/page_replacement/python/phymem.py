# This is the only file you must implement

# This file will be imported from the main code. The PhysicalMemory class
# will be instantiated with the algorithm received from the input. You may edit
# this file as you which

# NOTE: there may be methods you don't need to modify, you must decide what
# you need...

class PhysicalMemory:
  ALGORITHM_AGING_NBITS = 8
  """How many bits to use for the Aging algorithm"""

  def __init__(self, algorithm):
    assert algorithm in {"fifo", "nru", "aging", "second-chance"}
    self.algorithm = algorithm
    if(self.algorithm == "fifo"):
        self.policy = Fifo()
    elif(self.algorithm == "second-chance"):
        self.policy = Second_chance()

  def put(self, frameId):
    """Allocates this frameId for some page"""
    # Notice that in the physical memory we don't care about the pageId, we only
    # care about the fact we were requested to allocate a certain frameId
    self.policy.put(frameId)

  def evict(self):
    """Deallocates a frame from the physical memory and returns its frameId"""
    # You may assume the physical memory is FULL so we need space!
    # Your code must decide which frame to return, according to the algorithm
    return self.policy.evict()

  def clock(self):
    """The amount of time we set for the clock has passed, so this is called"""
    # Clear the reference bits (and/or whatever else you think you must do...)
    self.policy.clock()

  def access(self, frameId, isWrite):
    """A frameId was accessed for read/write (if write, isWrite=True)"""
    self.policy.access(frameId, isWrite)

class FrameMem:
    def __init__(self, frameId):
        self.id = frameId
        self.bit_R = False #Referenciado
        self.bit_M = False #Modificado


class Fifo:
    def __init__(self):
        self.queue = []

    def put(self, frameId):
        new_frame = FrameMem(frameId)
        self.queue.insert(0, new_frame)

    def evict(self):
        my_frame = self.queue.pop()
        return my_frame.id

    def clock(self):
        pass

    def access(self, frameId, isWrite):
        pass

class Second_chance:
    def __init__(self):
        self.queue = []

    def put(self, frameId):
        new_frame = FrameMem(frameId)
        self.queue.insert(0,new_frame)

    def evict(self):
        my_frame = self.queue.pop()
        if (my_frame.bit_R):
            my_frame.bit_R = False
            self.queue.insert(0,my_frame)
            return evict()
        return my_frame.id

    def clock(self):
        pass

    def access(self, frameId, isWrite):
        for frame in self.queue:
            if (frame.id == frameId):
                frame.R = True
