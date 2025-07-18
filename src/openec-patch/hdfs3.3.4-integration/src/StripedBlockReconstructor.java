/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.apache.hadoop.hdfs.server.datanode.erasurecode;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.apache.hadoop.classification.InterfaceAudience;
import org.apache.hadoop.hdfs.server.datanode.DataNodeFaultInjector;
import org.apache.hadoop.hdfs.server.datanode.metrics.DataNodeMetrics;
import org.apache.hadoop.io.erasurecode.rawcoder.InvalidDecodingException;
import org.apache.hadoop.util.Time;

/**
 * StripedBlockReconstructor reconstruct one or more missed striped block in
 * the striped block group, the minimum number of live striped blocks should
 * be no less than data block number.
 */
@InterfaceAudience.Private
class StripedBlockReconstructor extends StripedReconstructor
    implements Runnable {

  private StripedWriter stripedWriter;

  StripedBlockReconstructor(ErasureCodingWorker worker,
      StripedReconstructionInfo stripedReconInfo) {
    super(worker, stripedReconInfo);

    stripedWriter = new StripedWriter(this, getDatanode(),
        getConf(), stripedReconInfo);
  }

  boolean hasValidTargets() {
    return stripedWriter.hasValidTargets();
  }


  @Override
  public void run() {
    try {
      // benchmark single block repair time
      long startBlockRepairTime = System.currentTimeMillis();
      long endBlockRepairTime = 0;

      initDecoderIfNecessary();

      initDecodingValidatorIfNecessary();

      getStripedReader().init();

      stripedWriter.init();

      long startBlockReconstructTime = System.currentTimeMillis();
      long endBlockReconstructTime = 0;      

      reconstruct();

      endBlockReconstructTime = System.currentTimeMillis();
      long blockReconstructTime = endBlockReconstructTime - startBlockReconstructTime;
      System.out.println("StripedBlockReconstructor.blockReconstructTime: " + blockReconstructTime);


      stripedWriter.endTargetBlocks();

      // benchmark single block repair time
      endBlockRepairTime = System.currentTimeMillis();
      long blockRepairTime = endBlockRepairTime - startBlockRepairTime;
      System.out.println("StripedBlockReconstructor.blockRepairTime: " + blockRepairTime);

      // Currently we don't check the acks for packets, this is similar as
      // block replication.
    } catch (Throwable e) {
      LOG.warn("Failed to reconstruct striped block: {}", getBlockGroup(), e);
      getDatanode().getMetrics().incrECFailedReconstructionTasks();
    } finally {
      float xmitWeight = getErasureCodingWorker().getXmitWeight();
      // if the xmits is smaller than 1, the xmitsSubmitted should be set to 1
      // because if it set to zero, we cannot to measure the xmits submitted
      int xmitsSubmitted = Math.max((int) (getXmits() * xmitWeight), 1);
      getDatanode().decrementXmitsInProgress(xmitsSubmitted);
      final DataNodeMetrics metrics = getDatanode().getMetrics();
      metrics.incrECReconstructionTasks();
      metrics.incrECReconstructionBytesRead(getBytesRead());
      metrics.incrECReconstructionRemoteBytesRead(getRemoteBytesRead());
      metrics.incrECReconstructionBytesWritten(getBytesWritten());
      getStripedReader().close();
      stripedWriter.close();
      cleanup();
    }
  }

  @Override
  void reconstruct() throws IOException {
    
    long startTime = 0, endTime = 0;
    long RTime = 0, DTime = 0, WTime = 0;
    long RDTime = 0, RDWTime = 0;

    while (getPositionInBlock() < getMaxTargetLength()) {
      DataNodeFaultInjector.get().stripedBlockReconstruction();
      long remaining = getMaxTargetLength() - getPositionInBlock();
      final int toReconstructLen =
          (int) Math.min(getStripedReader().getBufferSize(), remaining);
 
      startTime = System.currentTimeMillis();

      long start = Time.monotonicNow();
      // step1: read from minimum source DNs required for reconstruction.
      // The returned success list is the source DNs we do real read from
      getStripedReader().readMinimumSources(toReconstructLen);
      long readEnd = Time.monotonicNow();

      endTime = System.currentTimeMillis();
      RTime += endTime - startTime;
      startTime = endTime;
 
      // step2: decode to reconstruct targets
      reconstructTargets(toReconstructLen);
      long decodeEnd = Time.monotonicNow();

      endTime = System.currentTimeMillis();
      DTime += endTime - startTime;
      startTime = endTime;
 
      // step3: transfer data
      if (stripedWriter.transferData2Targets() == 0) {
        String error = "Transfer failed for all targets.";
        throw new IOException(error);
      }
      long writeEnd = Time.monotonicNow();

      endTime = System.currentTimeMillis();
      WTime += endTime - startTime;
      startTime = endTime;
 
      // // benchmark time
      // long readTime = readEnd - start;
      // System.out.println("StripedBlockReconstructor.reconstruct.readTime: " + readTime);
 
      // long decodeTime = decodeEnd - readEnd;
      // System.out.println("StripedBlockReconstructor.reconstruct.decodeTime: " + decodeTime);
 
      // long writeTime = writeEnd - decodeEnd;
      // System.out.println("StripedBlockReconstructor.reconstruct.writeTime: " + writeTime);
 
      // long RNDTime = decodeEnd - start;
      // System.out.println("StripedBlockReconstructor.reconstruct.RNDTime: " + RNDTime);
 
      // long reconstructTime = writeEnd - start;
      // System.out.println("StripedBlockReconstructor.reconstruct.reconstructTime: " + reconstructTime);


      // Only the succeed reconstructions are recorded.
      final DataNodeMetrics metrics = getDatanode().getMetrics();
      metrics.incrECReconstructionReadTime(readEnd - start);
      metrics.incrECReconstructionDecodingTime(decodeEnd - readEnd);
      metrics.incrECReconstructionWriteTime(writeEnd - decodeEnd);

      updatePositionInBlock(toReconstructLen);

      clearBuffers();
    }

    // benchmark time
    RDTime = RTime + DTime;
    RDWTime = RDTime + WTime;

    System.out.println("StripedBlockReconstructor.reconstruct.RTime: " + RTime);
    System.out.println("StripedBlockReconstructor.reconstruct.DTime: " + DTime);
    System.out.println("StripedBlockReconstructor.reconstruct.WTime: " + WTime);
    System.out.println("StripedBlockReconstructor.reconstruct.RDTime: " + RDTime);
    System.out.println("StripedBlockReconstructor.reconstruct.RDWTime: " + RDWTime);
  }

  private void reconstructTargets(int toReconstructLen) throws IOException {
    ByteBuffer[] inputs = getStripedReader().getInputBuffers(toReconstructLen);

    int[] erasedIndices = stripedWriter.getRealTargetIndices();
    ByteBuffer[] outputs = stripedWriter.getRealTargetBuffers(toReconstructLen);

    if (isValidationEnabled()) {
      markBuffers(inputs);
      decode(inputs, erasedIndices, outputs);
      resetBuffers(inputs);

      DataNodeFaultInjector.get().badDecoding(outputs);
      long start = Time.monotonicNow();
      try {
        getValidator().validate(inputs, erasedIndices, outputs);
        long validateEnd = Time.monotonicNow();
        getDatanode().getMetrics().incrECReconstructionValidateTime(
            validateEnd - start);
      } catch (InvalidDecodingException e) {
        long validateFailedEnd = Time.monotonicNow();
        getDatanode().getMetrics().incrECReconstructionValidateTime(
            validateFailedEnd - start);
        getDatanode().getMetrics().incrECInvalidReconstructionTasks();
        throw e;
      }
    } else {
      decode(inputs, erasedIndices, outputs);
    }

    stripedWriter.updateRealTargetBuffers(toReconstructLen);
  }

  private void decode(ByteBuffer[] inputs, int[] erasedIndices,
      ByteBuffer[] outputs) throws IOException {
    long start = System.nanoTime();
    getDecoder().decode(inputs, erasedIndices, outputs);
    long end = System.nanoTime();
    this.getDatanode().getMetrics().incrECDecodingTime(end - start);
  }

  /**
   * Clear all associated buffers.
   */
  private void clearBuffers() {
    getStripedReader().clearBuffers();

    stripedWriter.clearBuffers();
  }
}
