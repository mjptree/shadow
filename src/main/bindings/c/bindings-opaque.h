/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */
// clang-format off


#ifndef main_opaque_bindings_h
#define main_opaque_bindings_h

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

typedef enum InterposeMethod {
  // Attach to child using ptrace and use it to interpose syscalls etc.
  INTERPOSE_METHOD_PTRACE,
  // Use LD_PRELOAD to load a library that implements the libC interface which will
  // route syscalls to Shadow.
  INTERPOSE_METHOD_PRELOAD,
} InterposeMethod;

typedef enum QDiscMode {
  Q_DISC_MODE_FIFO,
  Q_DISC_MODE_ROUND_ROBIN,
} QDiscMode;

// Memory allocated by Shadow, in a remote address space.
typedef struct AllocdMem_u8 AllocdMem_u8;

typedef struct Arc_AtomicRefCell_AbstractUnixNamespace Arc_AtomicRefCell_AbstractUnixNamespace;

// A queue of bytes that supports reading and writing stream and/or packet data.
//
// Both stream and packet data can be pushed onto the buffer and their order will be preserved.
// Data is stored internally as a linked list of chunks. Each chunk stores either stream or packet
// data. Consecutive stream data may be merged into a single chunk, but consecutive packets will
// always be contained in their own chunks.
//
// To avoid memory copies when moving bytes from one `ByteQueue` to another, you can use
// `pop_chunk()` to remove a chunk from the queue, and use `push_chunk()` to add it to another
// queue.
typedef struct ByteQueue ByteQueue;

// Utility for monitoring a set of child pid's, calling registered callbacks
// when one exits or is killed. Starts a background thread, which is shut down
// when the object is dropped.
typedef struct ChildPidWatcher ChildPidWatcher;

// Run real applications over simulated networks.
typedef struct CliOptions CliOptions;

typedef struct CompatDescriptor CompatDescriptor;

// Shadow configuration options after processing command-line and configuration file options.
typedef struct ConfigOptions ConfigOptions;

// The main counter object that maps individual keys to count values.
typedef struct Counter Counter;

// Table of (file) descriptors. Typically owned by a Process.
typedef struct DescriptorTable DescriptorTable;

typedef struct HostOptions HostOptions;

// Tool for assigning IP addresses to graph nodes.
typedef struct IpAssignment_u32 IpAssignment_u32;

// A set of `n` logical processors
typedef struct LogicalProcessors LogicalProcessors;

// Provides accessors for reading and writing another process's memory.
// When in use, any operation that touches that process's memory must go
// through the MemoryManager to ensure soundness. See MemoryManager::new.
typedef struct MemoryManager MemoryManager;

// A network graph containing the petgraph graph and a map from gml node ids to petgraph node
// indexes.
typedef struct NetworkGraph NetworkGraph;

// Represents a POSIX description, or a Linux "struct file".
typedef struct PosixFile PosixFile;

// A mutable reference to a slice of plugin memory. Implements DerefMut<[T]>,
// allowing, e.g.:
//
// let tpp = TypedPluginPtr::<u32>::new(ptr, 10);
// let pmr = memory_manager.memory_ref_mut(ptr);
// assert_eq!(pmr.len(), 10);
// pmr[5] = 100;
//
// The object must be disposed of by calling `flush` or `noflush`.  Dropping
// the object without doing so will result in a panic.
typedef struct ProcessMemoryRefMut_u8 ProcessMemoryRefMut_u8;

// An immutable reference to a slice of plugin memory. Implements Deref<[T]>,
// allowing, e.g.:
//
// let tpp = TypedPluginPtr::<u32>::new(ptr, 10);
// let pmr = memory_manager.memory_ref(ptr);
// assert_eq!(pmr.len(), 10);
// let x = pmr[5];
typedef struct ProcessMemoryRef_u8 ProcessMemoryRef_u8;

typedef struct ProcessOptions ProcessOptions;

typedef struct Random Random;

// Routing information for paths between nodes.
typedef struct RoutingInfo_u32 RoutingInfo_u32;

typedef struct StatusBar_ShadowStatusBarState StatusBar_ShadowStatusBarState;

#endif /* main_opaque_bindings_h */
