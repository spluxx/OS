package edu.duke.raft;

import java.util.Timer;

public class LeaderMode extends RaftMode {
  private int nextIndex[];
  private int matchIndex[];
  private Timer heartbeatTimer, pollingTimer;
  private final int POLLING_INTERVAL = 30;
  private final int POLLING_C = 100007;

  public void go () {
    synchronized (mLock) {
      System.out.println ("("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ": switched to leader mode.");
      nextIndex = new int[mConfig.getNumServers()+1];
      matchIndex = new int[mConfig.getNumServers()+1];
      for(int i = 0 ; i < nextIndex.length ; i ++) nextIndex[i] = mLog.getLastIndex()+1;
      for(int i = 1 ; i <= mConfig.getNumServers() ; i ++) remoteAppendEntries(i);
      resetTimer();
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      if(term < candidateTerm) { // WHAT! 
	mConfig.setCurrentTerm(candidateTerm, candidateID);
	heartbeatTimer.cancel();
	RaftServerImpl.setMode(new FollowerMode());
	return 0;
      } return term;
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      if(term < leaderTerm) { // WHAT! 
	mConfig.setCurrentTerm(leaderTerm, 0);
	heartbeatTimer.cancel();
	RaftServerImpl.setMode(new FollowerMode());
	return 0;
      } return term;
    }
  }

  private void remoteAppendEntries(int idx) {
    if(idx == mID) return;
    int term = mConfig.getCurrentTerm();
    int prevLogIndex = nextIndex[idx]-1;
    int prevLogTerm = prevLogIndex < 0 ? 0 : mLog.getEntry(nextIndex[idx]-1).term;
    Entry entries[] = new Entry[mLog.getLastIndex()-prevLogIndex];
    for(int i = prevLogIndex+1 ; i <= mLog.getLastIndex() ; i ++) {
      entries[i-prevLogIndex-1] = mLog.getEntry(i);
    }
    remoteAppendEntries(idx, term, mID, prevLogIndex, prevLogTerm, entries, mCommitIndex);
  }

  private void resetTimer() {
    if(heartbeatTimer != null) heartbeatTimer.cancel();
    if(pollingTimer != null) pollingTimer.cancel();
    heartbeatTimer = super.scheduleTimer(HEARTBEAT_INTERVAL, mID);
    pollingTimer = super.scheduleTimer(POLLING_INTERVAL, mID+POLLING_C);
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      if(mCommitIndex > mLastApplied) mLastApplied = mCommitIndex;
      int nServers = mConfig.getNumServers();
      int curTerm = mConfig.getCurrentTerm();

      if(timerID == POLLING_C+mID) { // process responses
	int[] responses = RaftResponses.getAppendResponses(curTerm);
	if(responses == null)  return;
	for(int i = 1 ; i <= nServers ; i ++) {
	  if(responses[i] < 0) continue;
	  else if(responses[i] == 0) { // YES!
	    System.out.println("("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ":" + "SERVER " + i + "returned 0 to appendRPC");
	    matchIndex[i] = mLog.getLastIndex(); 
	    nextIndex[i] = mLog.getLastIndex() + 1;
	  } else { // NO!
	    System.out.println("("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ":SERVER "+ i + "returned its term to appendRPC");
	    if(responses[i] > curTerm) { // STEP DOWN!
	      mConfig.setCurrentTerm(responses[i], 0); 
	      heartbeatTimer.cancel();
	      RaftServerImpl.setMode(new FollowerMode());
	      return;
	    } else nextIndex[i] --;
	  }
	}
	for(int i = mCommitIndex+1 ; i <= mLog.getLastIndex() ; i ++) {
	  int cnt = 0;
	  for(int j = 1 ; j <= mConfig.getNumServers() ; j ++) cnt += i <= matchIndex[j] ? 1 : 0;
	  if(cnt > mConfig.getNumServers()/2 && mLog.getEntry(i).term == curTerm) mCommitIndex = i;
	}
	RaftResponses.clearAppendResponses(curTerm);
      } else for(int i = 1 ; i <= nServers ; i ++) remoteAppendEntries(i); // boong boong
    }  
  }
}
