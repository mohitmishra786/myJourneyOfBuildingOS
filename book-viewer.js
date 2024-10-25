import React, { useState, useEffect } from 'react';
import { marked } from 'marked';

const isGitHubPages = window.location.hostname.includes('github.io');
const basePath = isGitHubPages ? '/myJourneyOfBuildingOS' : '';

const BookViewer = ({ bookConfig }) => {
  const [currentChapter, setCurrentChapter] = useState(0);
  const [currentPage, setCurrentPage] = useState(0);
  const [content, setContent] = useState('Loading...');
  const [error, setError] = useState(null);

  const loadContent = async () => {
    try {
      const currentPath = bookConfig.chapters[currentChapter].pages[currentPage].path;
      console.log('Loading content from:', currentPath);
      
      // Remove the repository name from the path if it's already included
      const cleanPath = currentPath.replace('/myJourneyOfBuildingOS', '');
      // Use relative path for local development, full path for GitHub Pages
      const finalPath = isGitHubPages ? basePath + cleanPath : cleanPath;
      
      const encodedPath = encodeURI(finalPath);
      const response = await fetch(encodedPath);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const text = await response.text();
      console.log('Content loaded, length:', text.length);
      
      setContent(marked.parse(text));
      setError(null);
    } catch (error) {
      console.error('Error loading content:', error);
      setError(error.message);
    }
  };

  useEffect(() => {
    loadContent();
  }, [currentChapter, currentPage]);

  const goToNextPage = () => {
    const currentChapterPages = bookConfig.chapters[currentChapter].pages.length;
    
    if (currentPage < currentChapterPages - 1) {
      setCurrentPage(prev => prev + 1);
    } else if (currentChapter < bookConfig.chapters.length - 1) {
      setCurrentChapter(prev => prev + 1);
      setCurrentPage(0);
    }
  };

  const goToPrevPage = () => {
    if (currentPage > 0) {
      setCurrentPage(prev => prev - 1);
    } else if (currentChapter > 0) {
      setCurrentChapter(prev => prev - 1);
      setCurrentPage(bookConfig.chapters[currentChapter - 1].pages.length - 1);
    }
  };

  const currentChapterData = bookConfig.chapters[currentChapter];
  const currentPageData = currentChapterData.pages[currentPage];
  const isFirstPage = currentChapter === 0 && currentPage === 0;
  const isLastPage = currentChapter === bookConfig.chapters.length - 1 && 
                    currentPage === bookConfig.chapters[currentChapter].pages.length - 1;

  return (
    <div className="max-w-4xl mx-auto p-4">
      <nav className="flex justify-between items-center mb-8">
        <button 
          onClick={goToPrevPage}
          disabled={isFirstPage}
          className={`px-4 py-2 bg-blue-500 text-white rounded ${isFirstPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}`}
        >
          ← Previous
        </button>
        
        <div className="text-lg font-semibold text-center">
          <div>Chapter {currentChapter + 1}: {currentChapterData.title}</div>
          <div className="text-sm text-gray-600 mt-1">{currentPageData.title}</div>
        </div>
        
        <button
          onClick={goToNextPage}
          disabled={isLastPage}
          className={`px-4 py-2 bg-blue-500 text-white rounded ${isLastPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}`}
        >
          Next →
        </button>
      </nav>

      <div className="prose max-w-none">
        {error ? (
          <div className="bg-red-100 border-l-4 border-red-500 text-red-700 p-4" role="alert">
            <p className="font-bold">Error loading content</p>
            <p>{error}</p>
          </div>
        ) : (
          <div dangerouslySetInnerHTML={{ __html: content }} />
        )}
      </div>

      <div className="mt-4 text-sm text-gray-500 text-center">
        Page {currentPage + 1} of {currentChapterData.pages.length}
      </div>
    </div>
  );
};

export default BookViewer;