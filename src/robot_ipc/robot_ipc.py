import ctypes
import pickle
import sys

from ._core import _HostFunctionCaller, _HostFunctionDispatcher, _HostVariable


_SIZE_FIELD_BYTES = ctypes.sizeof(ctypes.c_size_t)


def _validate_data_format(data_format):
    if data_format is None:
        return None
    if not isinstance(data_format, type) or not issubclass(
        data_format, ctypes.Structure
    ):
        raise TypeError("data_format must be a ctypes.Structure subclass")
    return data_format


def _struct_size(data_format):
    return ctypes.sizeof(data_format)


def _encode_data_format(data, data_format, target_size):
    if not isinstance(data, data_format):
        raise TypeError(f"data must be an instance of {data_format.__name__}")
    raw = ctypes.string_at(ctypes.addressof(data), ctypes.sizeof(data_format))
    if len(raw) > target_size:
        raise ValueError("data size exceeds target buffer size")
    return raw


def _decode_data_format(raw, data_format):
    return data_format.from_buffer_copy(raw[: ctypes.sizeof(data_format)])


def _encode_pickle_call(args, kwargs, target_size):
    args_data = pickle.dumps(args)
    kwargs_data = pickle.dumps(kwargs)
    payload = (
        len(args_data).to_bytes(_SIZE_FIELD_BYTES, sys.byteorder)
        + args_data
        + kwargs_data
    )
    if len(payload) > target_size:
        raise ValueError(f"encoded arguments exceed max size ({target_size})")
    return payload


def _decode_pickle_call(raw):
    args_size = int.from_bytes(raw[:_SIZE_FIELD_BYTES], sys.byteorder)
    args_start = _SIZE_FIELD_BYTES
    args_end = args_start + args_size
    args = pickle.loads(raw[args_start:args_end])
    kwargs = pickle.loads(raw[args_end:])
    return args, kwargs


class HostVariable:
    class _HostVariableProxy:
        def __get__(self, obj, objtype):
            return obj.read()

        def __set__(self, obj, value):
            obj.write(value)

    data = _HostVariableProxy()

    def __init__(self, name, max_size=4096, data_format=None):
        self.data_format = _validate_data_format(data_format)
        if self.data_format is not None:
            max_size = _struct_size(self.data_format)
        self.name = name
        self.max_size = max_size
        self._core = _HostVariable(name, max_size)

    def write(self, data, data_len=None):
        if self.data_format is None:
            payload = pickle.dumps(data)
            op_size = len(payload) if data_len is None else data_len
        else:
            payload = _encode_data_format(data, self.data_format, self.max_size)
            op_size = _struct_size(self.data_format) if data_len is None else data_len

        if op_size > self.max_size:
            raise ValueError(f"data length {op_size} exceeds max size {self.max_size}")
        self._core.write(payload, op_size)

    def read(self, data_len=None):
        op_size = self.max_size if data_len is None else data_len
        raw = self._core.read(op_size)
        if self.data_format is not None:
            return _decode_data_format(raw, self.data_format)
        return pickle.loads(raw)


class HostFunctionCaller:
    def __init__(
        self,
        name,
        max_arg_size=4096,
        max_ret_size=4096,
        arg_format=None,
        ret_format=None,
    ):
        self.arg_format = _validate_data_format(arg_format)
        self.ret_format = _validate_data_format(ret_format)

        if self.arg_format is not None:
            max_arg_size = _struct_size(self.arg_format)
        if self.ret_format is not None:
            max_ret_size = _struct_size(self.ret_format)

        self.max_arg_size = max_arg_size
        self.max_ret_size = max_ret_size
        self._core = _HostFunctionCaller(name, max_arg_size, max_ret_size)

    def __call__(self, *args, **kwargs):
        if self.arg_format is None:
            payload = _encode_pickle_call(args, kwargs, self.max_arg_size)
        else:
            if kwargs or len(args) != 1:
                raise ValueError(
                    "typed host function caller expects exactly one positional argument"
                )
            payload = _encode_data_format(args[0], self.arg_format, self.max_arg_size)
        self._core.call(payload)

    def get_response(self):
        raw = self._core.get_response()
        if self.ret_format is not None:
            return _decode_data_format(raw, self.ret_format)
        return pickle.loads(raw)


class HostFunctionDispatcher:
    def __init__(self, max_func_count=16):
        self._core = _HostFunctionDispatcher(max_func_count)
        self._callbacks = []

    def attach(
        self,
        name,
        foo,
        max_sz_args=4096,
        max_sz_ret=4096,
        arg_format=None,
        ret_format=None,
    ):
        arg_format = _validate_data_format(arg_format)
        ret_format = _validate_data_format(ret_format)

        if arg_format is not None:
            max_sz_args = _struct_size(arg_format)
        if ret_format is not None:
            max_sz_ret = _struct_size(ret_format)

        def _callback(raw):
            if arg_format is None:
                args, kwargs = _decode_pickle_call(raw)
                ret = foo(*args, **kwargs)
            else:
                ret = foo(_decode_data_format(raw, arg_format))

            if ret is None:
                return None

            if ret_format is None:
                payload = pickle.dumps(ret)
                if len(payload) > max_sz_ret:
                    raise ValueError(f"return data exceeds max_sz_ret ({max_sz_ret})")
                return payload

            return _encode_data_format(ret, ret_format, max_sz_ret)

        self._callbacks.append(_callback)
        self._core.attach(name, _callback, max_sz_args, max_sz_ret)

    def start(self):
        self._core.start()
