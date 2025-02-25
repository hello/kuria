# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: novelda.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='novelda.proto',
  package='novelda',
  serialized_pb=_b('\n\rnovelda.proto\x12\x07novelda\"E\n\nRadarFrame\x12\x10\n\x08\x66rame_id\x18\x01 \x01(\r\x12\x12\n\nrange_bins\x18\x02 \x03(\x01\x12\x11\n\tbase_band\x18\x03 \x01(\x08\x42\x1f\n\x16\x63om.hello.sati.noveldaB\x05Radar')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_RADARFRAME = _descriptor.Descriptor(
  name='RadarFrame',
  full_name='novelda.RadarFrame',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='frame_id', full_name='novelda.RadarFrame.frame_id', index=0,
      number=1, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='range_bins', full_name='novelda.RadarFrame.range_bins', index=1,
      number=2, type=1, cpp_type=5, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='base_band', full_name='novelda.RadarFrame.base_band', index=2,
      number=3, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=26,
  serialized_end=95,
)

DESCRIPTOR.message_types_by_name['RadarFrame'] = _RADARFRAME

RadarFrame = _reflection.GeneratedProtocolMessageType('RadarFrame', (_message.Message,), dict(
  DESCRIPTOR = _RADARFRAME,
  __module__ = 'novelda_pb2'
  # @@protoc_insertion_point(class_scope:novelda.RadarFrame)
  ))
_sym_db.RegisterMessage(RadarFrame)


DESCRIPTOR.has_options = True
DESCRIPTOR._options = _descriptor._ParseOptions(descriptor_pb2.FileOptions(), _b('\n\026com.hello.sati.noveldaB\005Radar'))
# @@protoc_insertion_point(module_scope)
