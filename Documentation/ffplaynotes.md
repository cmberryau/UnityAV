file type resolution:

parses options in cmdutils.c:377
(not able to resolve format?) av_find_input_format used to evaluate file format to AVInputFormat at ffplay.c:3574

global VideoState initialisation:

ffplay.c:stream_open

VideoState struct initialized at ffplay.c:3153
frame queues created using VideoState struct at ffplay.c:3164
packet queues created using VideoState struct at ffplay.c:3171
read thread created, VideoState struct passed in at ffplay.c:3188

running the main loop:

function read_thread has begun in a new thread at ffplay.c:2827
event_thread begins on the main thread

ffplay.c:read_thread

allocates a AVFormatContext struct at ffplay.c:2855
sets the interrupt callback for the AVFormatContext struct at ffplay.c:2861
opens the file and reads the header, setting the AVFormatContext struct members at ffplay.c:2867
sets the VideoState struct's ic member to the AVFormatContext struct at ffplay:2881
injects 'global side data' into the AVFormatContext struct at ffplay.c:2886
finds streams options at ffplay.c:2888
finds streams info at ffplay.c:2891
finds the best streams at ffplay.c:2950

opening streams from file:

opens the streams at ffplay.c:2978
opens video_thread, audio_thread and subtitle_thread at ffplay.c:2760

goes into packet reading loop at ffplay.c:3004

looks like pixels end up on screen in queue_picture at ffplay.c:1701