Example
-------

.. code-block:: python

    import pymusly as pm
    import random

    # create jukebox using defaults
    jukebox = pm.MuslyJukebox()

    # analyze tracks for usage with the jukebox using 30s second samples
    # from the middle of the track, but start at least at second 48
    filenames = [ ... ] # a list with paths to audio files
    tracks = [jukebox.track_from_audiofile(filename, length=30, start=-48)
        for filename in filenames]

    # use a representative (random) sample of your tracks to set the
    # style of the jukebox
    jukebox.set_style(random.sample(tracks, k=max(1000, len(tracks)/4)))

    # add all tracks to the jukebox
    track_ids = jukebox.add_tracks(tracks)

    # pick the first track as reference
    seed_track = (track_ids[0], tracks[0])

    # compute for all remaining tracks the similarity to the first track
    other_tracks = list(zip(track_ids[1:], tracks[1:]))
    similarities = jukebox.compute_similarity(seed_track, other_tracks)
    similarity_by_id = sorted(
        zip(track_ids[1:], similarities), reverse=True, key=lambda t: t[0])

    print(f"Similarities to track {filenames[0]}:")
    for id, value in similarity_by_id:
        print(f"  {filenames[id]} -> {value}")

