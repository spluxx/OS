package edu.duke.raft;

public abstract class RaftState {
  // latest term the server has seen
  protected static int mCurrentTerm;
  // candidate voted for in current term; 0, if none
  protected static int mVotedFor;
  // log containing server's entries
  protected static RaftLog mLog;
  // index of highest entry known to be committed
  protected static int mCommitIndex;
  // index of highest entry applied to state machine
  protected static int mLastApplied;


  public final void initialize (int currentTerm, 
				int votedFor,
				RaftLog log,
				int commitIndex,
				int lastApplied) {
    mCurrentTerm = currentTerm;
    mVotedFor = votedFor;
    mLog = log;
    mCommitIndex = commitIndex;    
    mLastApplied = lastApplied;
  }  

  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  abstract public int requestVote (int candidateTerm,
				   int candidateID,
				   int lastLogIndex,
				   int lastLogTerm);

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  abstract public int appendEntries (int leaderTerm,
				     int leaderID,
				     int prevLogIndex,
				     int prevLogTerm,
				     Entry[] entries,
				     int leaderCommit);

  // @param id of the timer that timed out
  abstract public void handleTimeout (int timerID);
}

  
