#include "timelinedock.h" 
#include "qmlview.h"
#include "qmlutilities.h"
#include "thumbnailprovider.h" 

#include <QQmlEngine>
#include <QQmlContext>
#include <QDockWidget>
#include <QQuickItem>
#include <QDir>
#include <QTime>

namespace timeline { 

	TimelineDock::TimelineDock(QWidget *parent)
		:mTimelineWidget(new Timeline(QmlUtilities::sharedEngine(), this)),
		mTimelineModel(new TimelineTracksModel), 
		mPosition(0),
		mSelection(0),
		mVisibleTickStep(2) {
		QDir importPath = QmlUtilities::qmlDir();
		mTimelineWidget->engine()->addImportPath(importPath.path());
		mTimelineWidget->engine()->addImageProvider(QString("thumbnail"), new ThumbnailProvider);
		QmlUtilities::registerCommonTypes();
		QmlUtilities::setCommonProperties(mTimelineWidget->rootContext());
		mTimelineWidget->rootContext()->setContextProperty("view", new QmlView(mTimelineWidget));
		mTimelineWidget->rootContext()->setContextProperty("TimelineWidget", this);
		mTimelineWidget->rootContext()->setContextProperty("TimelineModel", mTimelineModel);
		mTimelineWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
		mTimelineWidget->setClearColor(palette().window().color());
		mTimelineWidget->setFocusPolicy(Qt::StrongFocus); 
		setWidget(mTimelineWidget);
#ifdef Q_OS_WIN 
		onVisibilityChanged(true);
#else
		connect(this, &QDockWidget::visibilityChanged, this, &MainWindow::load);
#endif  
	}
	 
	void TimelineDock::onVisibilityChanged(bool visible) {
		if (visible) {
			load();
		}
	}

	void TimelineDock::load(bool force /* = false */) {
		if (mTimelineWidget->source().isEmpty() || force) {
			QDir sourcePath = QmlUtilities::qmlDir();
			mTimelineWidget->setSource(QUrl("qrc:/script/qml/timeline.qml"));
			if (force) {
				mTimelineModel->reload();
			}
		}
		else {
			mTimelineModel->reload();
		}
	}

	void TimelineDock::setPosition(int position) {
		if (position <= mTimelineModel->tracksAreaLength()) {
			mPosition = position;
		}
		else {
			mPosition = mTimelineModel->tracksAreaLength();
		}
		emit positionChanged();
	}

	void TimelineDock::setCurrentTrack(int currentTrack) {
		if (currentTrack < 0 || currentTrack >= 2) {
			mCurrentTrack = currentTrack;
		}
	}

	int TimelineDock::currentTrack() const {
		return mCurrentTrack;
	}

	void TimelineDock::setSelection(int selection) {
		mSelection = selection;
	}

	int TimelineDock::selection() const {
		return mSelection;
	}

	QString TimelineDock::timecode(int frames) { 
		int seconds = frames / mTimelineModel->referenceFrameRate();
		QTime time;
		time.setHMS(seconds / 3600, seconds / 60, seconds);
		return time.toString();
	} 

	void TimelineDock::addClip(int trackIndex) {
		qDebug() << "add clip " << trackIndex;
		if (trackIndex < 0) {
			trackIndex = currentTrack();
		}
		mTimelineModel->appendClip(trackIndex);
	} 

	void TimelineDock::copyClip(int trackIndex) {
		qDebug() << "copy clip: track->" << trackIndex;
		if (trackIndex < 0) {
			trackIndex = currentTrack();
		}
		mTimelineModel->copyClip(trackIndex, mSelection);
	}

	void TimelineDock::cutClip(int trackIndex, int clipIndex) {
		qDebug() << "cut clip: track->" << trackIndex << " clip->" << clipIndex;
	}

	void TimelineDock::splitClip(int trackIndex, int clipIndex) {
		qDebug() << "split clip: track->" << trackIndex << " position->" << mPosition;
		if (trackIndex < 0 || clipIndex < 0) {
			chooseClipAtPosition(mPosition, trackIndex, clipIndex);
		}
		if (trackIndex < 0 || clipIndex < 0) {
			qDebug() << "No available clip can be choosed";
			return;
		}
		setCurrentTrack(trackIndex);
		if (trackIndex >= 0 && clipIndex >= 0) {
			mTimelineModel->splitClip(trackIndex, clipIndex, mPosition);
		}
	}

	void TimelineDock::removeClip(int trackIndex, int clipIndex) {
		qDebug() << "remove clip: track->" << trackIndex << " clip->" << clipIndex;
		if (trackIndex < 0 || clipIndex < 0) {
			return;
		}
		mTimelineModel->removeClip(trackIndex, clipIndex);
	}

	void TimelineDock::chooseClipAtPosition(int position, int& trackIndex, int& clipIndex) {
		if (trackIndex >= 0) {
			clipIndex = clipIndexAtPosition(trackIndex, position);
			if (clipIndex >= 0) {
				return;
			}
		}

		trackIndex = currentTrack();
		clipIndex = clipIndexAtPosition(trackIndex, position);
		if (clipIndex >= 0) {
			return;
		}

		for (trackIndex = 0; trackIndex < mTimelineModel->tracksCount(); trackIndex++) {
			if (trackIndex == currentTrack()) {
				continue;
			}
			clipIndex = clipIndexAtPosition(trackIndex, position);
			if (clipIndex >= 0) {
				return;
			}
		}

		trackIndex = -1;
		clipIndex = -1;
	}

	int TimelineDock::clipIndexAtPosition(int trackIndex, int position) {
		int clipIndex = -1;
		if (trackIndex < 0) {
			trackIndex = currentTrack();
		}
		if (trackIndex >= 0 && trackIndex < mTimelineModel->tracksCount()) {
			clipIndex = mTimelineModel->getClipIndexAt(trackIndex, position);
		}

		return clipIndex;
	}

	int TimelineDock::clipIndexAtPlayhead(int trackIndex) {
		return clipIndexAtPosition(trackIndex, mPosition);
	}

	bool TimelineDock::getClipInfo(int trackIndex, int clipIndex, ClipInfo& clipInfo) {
		if (trackIndex < 0 || trackIndex > mTimelineModel->tracksCount()
			|| clipIndex < 0) {
			return false;
		}
		return mTimelineModel->getClipInfo(trackIndex, clipIndex, clipInfo);
	}
}


