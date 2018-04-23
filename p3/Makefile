RAFT_PATH=edu/duke/raft
SRC_DIR=src/$(RAFT_PATH)
JC = javac
JFLAGS = -d ./bin -classpath ./bin:./src

.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = \
	$(SRC_DIR)/CandidateMode.java \
	$(SRC_DIR)/Entry.java \
	$(SRC_DIR)/FollowerMode.java \
	$(SRC_DIR)/LeaderMode.java \
	$(SRC_DIR)/RaftConfig.java \
	$(SRC_DIR)/RaftLog.java \
	$(SRC_DIR)/RaftMode.java \
	$(SRC_DIR)/RaftResponses.java \
	$(SRC_DIR)/RaftServer.java \
	$(SRC_DIR)/RaftServerImpl.java \
	$(SRC_DIR)/StartClient.java \
	$(SRC_DIR)/StartServer.java

default: classes

classes: $(CLASSES:.java=.class)

test: 
	$(RM) ./serverlogs/1.config
	$(RM) ./serverlogs/2.config
	$(RM) ./serverlogs/3.config
	$(RM) ./serverlogs/server.output
	cp ./serverlogs/1c.config ./serverlogs/1.config
	cp ./serverlogs/2c.config ./serverlogs/2.config
	cp ./serverlogs/3c.config ./serverlogs/3.config
	./bin/runtest.sh ${ARGS}

easytest:
	make ARGS="10 tests/easytest.txt" test 
sampletest:
	make ARGS="10 tests/sampletest.txt" test 

clean:
	$(RM) ./bin/$(RAFT_PATH)/*.class
